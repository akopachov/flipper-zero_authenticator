#include "bt_type_code.h"
#include <furi_hal_bt_hid.h>
#include <storage/storage.h>
#include "../../types/common.h"
#include "../../services/convert/convert.h"

#define HID_BT_KEYS_STORAGE_PATH EXT_PATH("authenticator/.bt_hid.keys")

static const uint8_t hid_number_keys[10] = {
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

static inline bool totp_type_code_worker_stop_requested() {
    return furi_thread_flags_get() & TotpBtTypeCodeWorkerEventStop;
}

static void totp_type_code_worker_type_code(TotpBtTypeCodeWorkerContext* context) {
    uint8_t i = 0;
    do {
        furi_delay_ms(500);
        i++;
    } while(!furi_hal_bt_is_active() && i < 100 && !totp_type_code_worker_stop_requested());

    if(furi_hal_bt_is_active() &&
       furi_mutex_acquire(context->string_sync, 500) == FuriStatusOk) {
        furi_delay_ms(500);
        i = 0;
        while(i < context->string_length && context->string[i] != 0) {
            uint8_t digit = CONVERT_CHAR_TO_DIGIT(context->string[i]);
            if(digit > 9) break;
            uint8_t hid_kb_key = hid_number_keys[digit];
            furi_hal_bt_hid_kb_press(hid_kb_key);
            furi_delay_ms(30);
            furi_hal_bt_hid_kb_release(hid_kb_key);
            i++;
        }

        furi_mutex_release(context->string_sync);

        furi_delay_ms(100);
    }
}

static int32_t totp_type_code_worker_callback(void* context) {
    ValueMutex context_mutex;
    if(!init_mutex(&context_mutex, context, sizeof(TotpBtTypeCodeWorkerContext))) {
        return 251;
    }

    TotpBtTypeCodeWorkerContext *t_context = context;

    t_context->bt = furi_record_open(RECORD_BT);
    bt_disconnect(t_context->bt);
    furi_hal_bt_reinit();
    furi_delay_ms(200);
    bt_keys_storage_set_storage_path(t_context->bt, HID_BT_KEYS_STORAGE_PATH);
    if(!bt_set_profile(t_context->bt, BtProfileHidKeyboard)) {
        FURI_LOG_E(LOGGING_TAG, "Failed to switch BT to keyboard HID profile");
    }

    furi_hal_bt_start_advertising();
    while(true) {
        uint32_t flags = furi_thread_flags_wait(
            TotpBtTypeCodeWorkerEventStop | TotpBtTypeCodeWorkerEventType,
            FuriFlagWaitAny,
            FuriWaitForever);
        furi_check((flags & FuriFlagError) == 0); //-V562
        if(flags & TotpBtTypeCodeWorkerEventStop) break;

        TotpBtTypeCodeWorkerContext* h_context = acquire_mutex_block(&context_mutex);
        if(flags & TotpBtTypeCodeWorkerEventType) {
            totp_type_code_worker_type_code(h_context);
        }

        release_mutex(&context_mutex, h_context);
    }

    bt_disconnect(t_context->bt);
    furi_delay_ms(200);
    bt_keys_storage_set_default_path(t_context->bt);

    if(!bt_set_profile(t_context->bt, BtProfileSerial)) {
        FURI_LOG_E(LOGGING_TAG, "Failed to switch BT to Serial profile");
    }
    furi_record_close(RECORD_BT);

    delete_mutex(&context_mutex);

    return 0;
}

TotpBtTypeCodeWorkerContext* totp_bt_type_code_worker_start() {
    TotpBtTypeCodeWorkerContext* context = malloc(sizeof(TotpBtTypeCodeWorkerContext));
    furi_check(context != NULL);
    context->string_sync = furi_mutex_alloc(FuriMutexTypeNormal);
    context->thread = furi_thread_alloc();
    furi_thread_set_name(context->thread, "TOTPBtHidWorker");
    furi_thread_set_stack_size(context->thread, 1024);
    furi_thread_set_context(context->thread, context);
    furi_thread_set_callback(context->thread, totp_type_code_worker_callback);
    furi_thread_start(context->thread);

    return context;
}

void totp_bt_type_code_worker_stop(TotpBtTypeCodeWorkerContext* context) {
    furi_assert(context != NULL);
    furi_thread_flags_set(furi_thread_get_id(context->thread), TotpBtTypeCodeWorkerEventStop);
    furi_thread_join(context->thread);
    furi_thread_free(context->thread);
    furi_mutex_free(context->string_sync);
    free(context);
}

void totp_bt_type_code_worker_notify(TotpBtTypeCodeWorkerContext* context, TotpBtTypeCodeWorkerEvent event) {
    furi_assert(context != NULL);
    furi_thread_flags_set(furi_thread_get_id(context->thread), event);
}