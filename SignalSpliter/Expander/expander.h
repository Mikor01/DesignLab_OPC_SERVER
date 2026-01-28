#ifndef __EXPANDER_H__
#define __EXPANDER_H__

#include <pico/stdlib.h>
#include <hardware/i2c.h>

#include "i2c.h"

#define EXPANDER_SEQUENTIAL_BANK    true
#define EXPANDER_IRQ_GPIO_OD        false
#define EXPANDER_IRQ_POLARITY       true

typedef struct Expander {
    i2c_inst_t *channel;
    uint8_t address;
    uint8_t reset_gpio;
} Expander_t;


Expander_t EXPANDER_init(i2c_inst_t *I2C_channel, uint8_t device_address, uint8_t reset_gpio);
int EXPANDER_config(Expander_t *expander);
void EXPANDER_reset(Expander_t *expander);


/*! \brief Set mask GPIO direction
 *  \ingroup expander
 *
 * \param dir 0 for out, 1 for in
 */
int EXPANDER_set_dir(Expander_t *expander, uint16_t dir);


/*! \brief Set mask GPIO output value
 *  \ingroup expander
 *
 * \param out If false clear the GPIO, otherwise set it.
 */
int EXPANDER_put(Expander_t *expander, uint16_t out);



// /*! \brief Set mask GPIO INPUT polarity
//  *  \ingroup expander
//  *
//  * \param polarity 1 for invert logic
//  */
// int EXPANDER_set_polarity(Expander_t *expander, uint16_t polarity);
// 
// 
// /*! \brief Set GPIO irq mask
//  *  \ingroup expander
//  *
//  * \param irq 1 for enable
//  */
// int EXPANDER_set_irq_on_change(Expander_t *expander, uint16_t irq);

int EXPANDER_put_A(Expander_t *expander, uint8_t out);
int EXPANDER_put_B(Expander_t *expander, uint8_t out);
int EXPANDER_set_dir_A(Expander_t *expander, uint8_t dir);
int EXPANDER_set_dir_B(Expander_t *expander, uint8_t dir);


#endif