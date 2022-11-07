#include "hid_worker.h"

const uint8_t hid_number_keys[10] = {
    HID_KEYBOARD_0,
    HID_KEYBOARD_1,
    HID_KEYBOARD_2,
    HID_KEYBOARD_3,
    HID_KEYBOARD_4,
    HID_KEYBOARD_5,
    HID_KEYBOARD_6,
    HID_KEYBOARD_7,
    HID_KEYBOARD_8,
    HID_KEYBOARD_9};

static void totp_hid_worker_type_code(TotpHidWorkerTypeContext* context) {
    FuriHalUsbInterface* usb_mode_prev = furi_hal_usb_get_config();
    furi_hal_usb_unlock();
    furi_check(furi_hal_usb_set_config(&usb_hid, NULL) == true);
    uint8_t i = 0;
    do {
        furi_delay_ms(500);
        i++;
    } while(!furi_hal_hid_is_connected() && i < 100);

    if (furi_hal_hid_is_connected()) {
        uint8_t str_to_type_length = context->string_length;
        char* str_to_type = malloc(str_to_type_length + 1);
        if (str_to_type != NULL) {
            strlcpy(str_to_type, context->string, str_to_type_length + 1);
            i = 0;
            while(i < str_to_type_length && str_to_type[i] != 0) {
                uint8_t digit = str_to_type[i] - '0';
                if (digit > 9) break;
                uint8_t hid_kb_key = hid_number_keys[digit];
                furi_hal_hid_kb_press(hid_kb_key);
                furi_delay_ms(30);
                furi_hal_hid_kb_release(hid_kb_key);
                i++;
            }

            free(str_to_type);
        }

        furi_hal_hid_kb_press(HID_KEYBOARD_RETURN);
        furi_delay_ms(30);
        furi_hal_hid_kb_release(HID_KEYBOARD_RETURN);

        furi_delay_ms(100);
    }

    furi_hal_usb_set_config(usb_mode_prev, NULL);
}

static int32_t totp_hid_worker_callback(void* context) {
    ValueMutex context_mutex;
    if (!init_mutex(&context_mutex, context, sizeof(TotpHidWorkerTypeContext))) {
        return 251;
    }

    while(true) {
        uint32_t flags = furi_thread_flags_wait(
            TotpHidWorkerEvtStop | TotpHidWorkerEvtType, FuriFlagWaitAny, FuriWaitForever);
        furi_check((flags & FuriFlagError) == 0); //-V562
        if(flags & TotpHidWorkerEvtStop) break;

        TotpHidWorkerTypeContext* h_context = acquire_mutex_block(&context_mutex);
        if(flags & TotpHidWorkerEvtType) {
            totp_hid_worker_type_code(h_context);
        }

        release_mutex(&context_mutex, h_context);
    }

    delete_mutex(&context_mutex);

    return 0;
}

TotpHidWorkerTypeContext* totp_hid_worker_start() {
    TotpHidWorkerTypeContext* context = malloc(sizeof(TotpHidWorkerTypeContext));
    furi_check(context != NULL);
    context->thread = furi_thread_alloc();
    furi_thread_set_name(context->thread, "TOTPHidWorker");
    furi_thread_set_stack_size(context->thread, 1024);
    furi_thread_set_context(context->thread, context);
    furi_thread_set_callback(context->thread, totp_hid_worker_callback);
    furi_thread_start(context->thread);
    return context;
}

void totp_hid_worker_stop(TotpHidWorkerTypeContext* context) {
    furi_assert(context);
    furi_thread_flags_set(furi_thread_get_id(context->thread), TotpHidWorkerEvtStop);
    furi_thread_join(context->thread);
    furi_thread_free(context->thread);
    free(context);
}

void totp_hid_worker_notify(TotpHidWorkerTypeContext* context, TotpHidWorkerEvtFlags event) {
    furi_assert(context);
    furi_thread_flags_set(furi_thread_get_id(context->thread), event);
}