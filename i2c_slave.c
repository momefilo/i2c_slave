/*
 * Copyright (c) 2021 Valentin Milea <valentin.milea@gmail.com>
 * Copyright (c) 2023 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * Edit by momefilo for micropython
 */
#include "py/runtime.h"
#include "include/pico/i2c_slave.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include <string.h>

mp_obj_t callback_obj;

typedef struct i2c_slave {
    i2c_slave_handler_t handler;
    bool transfer_in_progress;
} i2c_slave_t;

static i2c_slave_t i2c_slaves[2];
// Die callback-Funktion
static void __isr __not_in_flash_func(i2c_slave_irq_handler)(void) {
    uint i2c_index = __get_current_exception() - VTABLE_FIRST_IRQ - I2C0_IRQ;
    i2c_slave_t *slave = &i2c_slaves[i2c_index];
    i2c_inst_t *i2c = i2c_get_instance(i2c_index);
    i2c_hw_t *hw = i2c_get_hw(i2c);

    extern mp_obj_t callback_obj;
    
    uint32_t intr_stat = hw->intr_stat;
    if (intr_stat == 0) {
        return;
    }
    bool do_finish_transfer = false;
    if (intr_stat & I2C_IC_INTR_STAT_R_TX_ABRT_BITS) {
        hw->clr_tx_abrt;
        do_finish_transfer = true;
    }
    if (intr_stat & I2C_IC_INTR_STAT_R_START_DET_BITS) {
        hw->clr_start_det;
        do_finish_transfer = true;
    }
    if (intr_stat & I2C_IC_INTR_STAT_R_STOP_DET_BITS) {
        hw->clr_stop_det;
        do_finish_transfer = true;
    }
    if (do_finish_transfer && slave->transfer_in_progress) {
        mp_obj_t ret = mp_obj_new_str("I2C_SLAVE_FINISH", strlen("I2C_SLAVE_FINISH"));
        mp_call_function_2(callback_obj, i2c, ret);
//        slave->handler(i2c, I2C_SLAVE_FINISH);
        slave->transfer_in_progress = false;
    }
    if (intr_stat & I2C_IC_INTR_STAT_R_RX_FULL_BITS) {
        slave->transfer_in_progress = true;
        mp_obj_t ret = mp_obj_new_str("I2C_SLAVE_RECEIVE", strlen("I2C_SLAVE_RECEIVE"));
        mp_call_function_2((callback_obj), i2c, ret);
//        slave->handler(i2c, I2C_SLAVE_RECEIVE);
    }
    if (intr_stat & I2C_IC_INTR_STAT_R_RD_REQ_BITS) {
        hw->clr_rd_req;
        slave->transfer_in_progress = true;
        mp_obj_t ret = mp_obj_new_str("I2C_SLAVE_REQUEST", strlen("I2C_SLAVE_REQUEST"));
        mp_call_function_2(callback_obj, i2c, ret);
//        slave->handler(i2c, I2C_SLAVE_REQUEST);
    }
}
static mp_obj_t _i2c_slave_irq_handler(void) {
    return MP_OBJ_FROM_PTR(&i2c_slave_irq_handler);
}
MP_DEFINE_CONST_FUN_OBJ_0(i2c_slave_irq_handler_obj, _i2c_slave_irq_handler);

// Die readfunktion
static mp_obj_t _i2c_read(mp_obj_t i2c) {
    i2c_inst_t *_i2c = MP_OBJ_TO_PTR(i2c);
    return mp_obj_new_int(i2c_read_byte_raw((_i2c)));
}
MP_DEFINE_CONST_FUN_OBJ_1(i2c_read_obj, _i2c_read);

// Die writefunktion
static mp_obj_t _i2c_write(mp_obj_t i2c, mp_obj_t data) {
    i2c_inst_t *_i2c = MP_OBJ_TO_PTR(i2c);
    uint8_t _data = mp_obj_get_int(data);
    i2c_write_byte_raw(_i2c, _data);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(i2c_write_obj, _i2c_write);

// Die init-Funktion
void i2c_slave_init(i2c_inst_t *i2c, uint8_t address, i2c_slave_handler_t handler) {
    assert(i2c == i2c0 || i2c == i2c1);
    assert(handler != NULL);

    uint i2c_index = i2c_hw_index(i2c);
    i2c_slave_t *slave = &i2c_slaves[i2c_index];
    slave->handler = handler;

    // Note: The I2C slave does clock stretching implicitly after a RD_REQ, while the Tx FIFO is empty.
    // Clock stretching while the Rx FIFO is full is also enabled by default.
    i2c_set_slave_mode(i2c, true, address);

    i2c_hw_t *hw = i2c_get_hw(i2c);
    // unmask necessary interrupts
    hw->intr_mask =
            I2C_IC_INTR_MASK_M_RX_FULL_BITS | I2C_IC_INTR_MASK_M_RD_REQ_BITS | I2C_IC_INTR_MASK_M_TX_ABRT_BITS |
            I2C_IC_INTR_MASK_M_STOP_DET_BITS | I2C_IC_INTR_MASK_M_START_DET_BITS;

    // enable interrupt for current core
    uint num = I2C0_IRQ + i2c_index;
    irq_set_exclusive_handler(num, i2c_slave_irq_handler);
    irq_set_enabled(num, true);
    printf("i2c_slave.init(%d)\n", i2c_index);
}
static mp_obj_t _i2c_slave_init(size_t n_args, const mp_obj_t *args) {
    uint8_t _i2c = mp_obj_get_int(args[0]);
    uint8_t _sda = mp_obj_get_int(args[1]);
    uint8_t _scl = mp_obj_get_int(args[2]);
    uint _bdrate = mp_obj_get_int(args[3]);
    uint8_t _address = mp_obj_get_int(args[4]);
    
    i2c_slave_handler_t _handler = MP_OBJ_TO_PTR(args[5]);
//    MP_REGISTER_ROOT_POINTER(mp_obj_t callback_obj);
    callback_obj = args[5];
    
    gpio_init(_sda);
    gpio_set_function(_sda, GPIO_FUNC_I2C);
    gpio_pull_up(_sda);

    gpio_init(_scl);
    gpio_set_function(_scl, GPIO_FUNC_I2C);
    gpio_pull_up(_scl);
    
    i2c_init(I2C_INSTANCE(_i2c), _bdrate);
    i2c_slave_init(I2C_INSTANCE(_i2c), _address, _handler);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(i2c_slave_init_obj, 6, 6, _i2c_slave_init);

// Die deinit-Funktion
void i2c_slave_deinit(i2c_inst_t *i2c) {
    assert(i2c == i2c0 || i2c == i2c1);

    uint i2c_index = i2c_hw_index(i2c);
    i2c_slave_t *slave = &i2c_slaves[i2c_index];
    assert(slave->handler); // should be called after i2c_slave_init()

    slave->handler = NULL;
    slave->transfer_in_progress = false;

    uint num = I2C0_IRQ + i2c_index;
    irq_set_enabled(num, false);
    irq_remove_handler(num, i2c_slave_irq_handler);

    i2c_hw_t *hw = i2c_get_hw(i2c);
    hw->intr_mask = I2C_IC_INTR_MASK_RESET;

    i2c_set_slave_mode(i2c, false, 0);
    i2c_deinit(i2c);
}
static mp_obj_t _i2c_slave_deinit(mp_obj_t i2c) {
    i2c_slave_deinit(I2C_INSTANCE((mp_obj_get_int(i2c))));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(i2c_slave_deinit_obj, _i2c_slave_deinit);

// Export zu micropython
static const mp_rom_map_elem_t i2c_slave_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_i2c_slave) },
    { MP_ROM_QSTR(MP_QSTR_irq_handler), MP_ROM_PTR(&i2c_slave_irq_handler_obj) },
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&i2c_slave_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&i2c_slave_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&i2c_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&i2c_write_obj) },
};
static MP_DEFINE_CONST_DICT(i2c_slave_module_globals, i2c_slave_module_globals_table);

// Define module object.
const mp_obj_module_t i2c_slave_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&i2c_slave_module_globals,
};

// Register the module to make it available in Python.
MP_REGISTER_MODULE(MP_QSTR_i2c_slave, i2c_slave_user_cmodule);
