#include "uhf_worker.h"
#include "uhf_tag.h"

#define MAX_READ_WORDS 8
#define MAX_READ_ATTEMPTS 10

UHFWorkerEvent verify_module_connected(UHFWorker* uhf_worker) {
    char* hw_version = m100_get_hardware_version(uhf_worker->module);
    char* sw_version = m100_get_software_version(uhf_worker->module);
    char* manufacturer = m100_get_manufacturers(uhf_worker->module);
    if(!hw_version || !sw_version || !manufacturer) return UHFWorkerEventFail;
    return UHFWorkerEventSuccess;
}

UHFTag* send_polling_command(UHFWorker* uhf_worker) {
    UHFTag* uhf_tag = uhf_tag_alloc();
    if(!uhf_tag) return NULL;
    M100ResponseType status;
    int attempts = 0;
    do {
        if(uhf_worker->state == UHFWorkerStateStop) {
            uhf_tag_free(uhf_tag);
            return NULL;
        }
        status = m100_single_poll(uhf_worker->module, uhf_tag);
        attempts++;
        if(attempts > MAX_READ_ATTEMPTS) {
            uhf_tag_free(uhf_tag);
            return NULL;
        }
    } while(status != M100SuccessResponse);
    return uhf_tag;
}

UHFWorkerEvent read_bank_limited(UHFWorker* uhf_worker, UHFTag* uhf_tag, BankType bank) {
    if(!uhf_tag) return UHFWorkerEventFail;
    M100ResponseType status;
    int attempts = 0;
    status = m100_read_label_data_storage(uhf_worker->module, uhf_tag, bank, 0, MAX_READ_WORDS);
    while(status != M100SuccessResponse) {
        if(uhf_worker->state == UHFWorkerStateStop) return UHFWorkerEventAborted;
        attempts++;
        if(attempts > MAX_READ_ATTEMPTS) return UHFWorkerEventFail;
        status = m100_read_label_data_storage(uhf_worker->module, uhf_tag, bank, 0, MAX_READ_WORDS);
    }
    return UHFWorkerEventSuccess;
}

UHFWorkerEvent read_single_card(UHFWorker* uhf_worker) {
    UHFTag* uhf_tag = send_polling_command(uhf_worker);
    if(!uhf_tag) return UHFWorkerEventAborted;
    uhf_tag_wrapper_set_tag(uhf_worker->uhf_tag_wrapper, uhf_tag);
    int attempts = 0;
    while(m100_set_select(uhf_worker->module, uhf_tag) != M100SuccessResponse) {
        if(uhf_worker->state == UHFWorkerStateStop) return UHFWorkerEventAborted;
        attempts++;
        if(attempts > MAX_READ_ATTEMPTS) return UHFWorkerEventFail;
    }
    UHFWorkerEvent event;
    event = read_bank_limited(uhf_worker, uhf_tag, EPCBank);
    if(event != UHFWorkerEventSuccess) return event;
    event = read_bank_limited(uhf_worker, uhf_tag, TIDBank);
    if(event != UHFWorkerEventSuccess) return event;
    event = read_bank_limited(uhf_worker, uhf_tag, UserBank);
    if(event != UHFWorkerEventSuccess) return event;
    return UHFWorkerEventSuccess;
}

UHFWorkerEvent write_single_card(UHFWorker* uhf_worker) {
    UHFTag* uhf_tag_des = send_polling_command(uhf_worker);
    if(!uhf_tag_des) return UHFWorkerEventAborted;
    UHFTag* uhf_tag_from = uhf_worker->uhf_tag_wrapper->uhf_tag;
    M100ResponseType rp_type;
    int attempts;
    attempts = 0;
    do {
        rp_type = m100_set_select(uhf_worker->module, uhf_tag_des);
        if(uhf_worker->state == UHFWorkerStateStop) return UHFWorkerEventAborted;
        if(rp_type == M100SuccessResponse) break;
        attempts++;
        if(attempts > MAX_READ_ATTEMPTS) return UHFWorkerEventFail;
    } while(1);
    attempts = 0;
    while(m100_is_write_mask_enabled(uhf_worker->module, WRITE_USER)) {
        rp_type = m100_write_label_data_storage(uhf_worker->module, uhf_tag_from, uhf_tag_des, UserBank, 0, 0);
        if(uhf_worker->state == UHFWorkerStateStop) return UHFWorkerEventAborted;
        if(rp_type == M100SuccessResponse) break;
        attempts++;
        if(attempts > MAX_READ_ATTEMPTS) return UHFWorkerEventFail;
    }
    attempts = 0;
    while(m100_is_write_mask_enabled(uhf_worker->module, WRITE_TID)) {
        rp_type = m100_write_label_data_storage(uhf_worker->module, uhf_tag_from, uhf_tag_des, TIDBank, 0, 0);
        if(uhf_worker->state == UHFWorkerStateStop) return UHFWorkerEventAborted;
        if(rp_type == M100SuccessResponse) break;
        attempts++;
        if(attempts > MAX_READ_ATTEMPTS) return UHFWorkerEventFail;
    }
    attempts = 0;
    while(m100_is_write_mask_enabled(uhf_worker->module, WRITE_EPC)) {
        rp_type = m100_write_label_data_storage(uhf_worker->module, uhf_tag_from, uhf_tag_des, EPCBank, 0, 0);
        if(uhf_worker->state == UHFWorkerStateStop) return UHFWorkerEventAborted;
        if(rp_type == M100SuccessResponse) break;
        attempts++;
        if(attempts > MAX_READ_ATTEMPTS) return UHFWorkerEventFail;
    }
    return UHFWorkerEventSuccess;
}

int32_t uhf_worker_task(void* ctx) {
    UHFWorker* uhf_worker = ctx;
    if(uhf_worker->state == UHFWorkerStateVerify) {
        UHFWorkerEvent event = verify_module_connected(uhf_worker);
        uhf_worker->callback(event, uhf_worker->ctx);
    } else if(uhf_worker->state == UHFWorkerStateDetectSingle) {
        UHFWorkerEvent event = read_single_card(uhf_worker);
        uhf_worker->callback(event, uhf_worker->ctx);
    } else if(uhf_worker->state == UHFWorkerStateWriteSingle) {
        UHFWorkerEvent event = write_single_card(uhf_worker);
        uhf_worker->callback(event, uhf_worker->ctx);
    }
    return 0;
}

UHFWorker* uhf_worker_alloc() {
    UHFWorker* uhf_worker = (UHFWorker*)malloc(sizeof(UHFWorker));
    uhf_worker->thread = furi_thread_alloc_ex("UHFWorker", UHF_WORKER_STACK_SIZE, uhf_worker_task, uhf_worker);
    uhf_worker->module = m100_module_alloc();
    uhf_worker->callback = NULL;
    uhf_worker->ctx = NULL;
    return uhf_worker;
}

void uhf_worker_change_state(UHFWorker* worker, UHFWorkerState state) {
    worker->state = state;
}

void uhf_worker_start(UHFWorker* uhf_worker, UHFWorkerState state, UHFWorkerCallback callback, void* ctx) {
    uhf_worker->state = state;
    uhf_worker->callback = callback;
    uhf_worker->ctx = ctx;
    furi_thread_start(uhf_worker->thread);
}

void uhf_worker_stop(UHFWorker* uhf_worker) {
    furi_assert(uhf_worker);
    furi_assert(uhf_worker->thread);
    if(furi_thread_get_state(uhf_worker->thread) != FuriThreadStateStopped) {
        uhf_worker_change_state(uhf_worker, UHFWorkerStateStop);
        furi_thread_join(uhf_worker->thread);
    }
}

void uhf_worker_free(UHFWorker* uhf_worker) {
    furi_assert(uhf_worker);
    furi_thread_free(uhf_worker->thread);
    m100_module_free(uhf_worker->module);
    free(uhf_worker);
}