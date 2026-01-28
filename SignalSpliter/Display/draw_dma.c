#include "draw_dma.h"
#include "comman.h"
#include "draw.h"
#include "display.h"

#include <stdio.h>
#include <hardware/dma.h>

#include "spi.h"
#include "interface.h"

static uint channel;

static inline uint dma_getCurrentIndex(uint dmaChannel){
    dma_channel_hw_t *channel = dma_channel_hw_addr(dmaChannel);
    uint transfer_count = channel->al3_transfer_count;
    return 8192 - transfer_count;
}



void dma_finish_write(){
    dma_hw->ints0 = 1 << channel;
    // dma_channel_abort(channel);
    SPI_select(DISPLAY_CS_GPIO);
}


void DRAW_DMA_Init(uint8_t *tx_buffer, uint size){
    channel = dma_claim_unused_channel(true);
    dma_channel_config config = dma_channel_get_default_config(channel);

    channel_config_set_transfer_data_size(&config, DMA_SIZE_8);
    channel_config_set_dreq(&config, spi_get_dreq(SPI_CHANNEL, true));
    channel_config_set_read_increment(&config, true);
    channel_config_set_write_increment(&config, false);
    dma_channel_set_write_addr(channel, &(spi_get_hw(SPI_CHANNEL)->dr), false);
    dma_channel_set_config(channel, &config, false);

    dma_channel_set_irq0_enabled(channel, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_finish_write);
    irq_set_enabled(DMA_IRQ_0, true);
}

bool send_buffer_to_OLED_via_dma(uint8_t *buffer){
    if(dma_channel_is_busy(channel)) return false;

    DISPLAY_setWindow(0, 63, 0, 127);
    write_command(WRITE_RAM_COMMAND);
    data_mode();

    SPI_select(DISPLAY_CS_GPIO);

    dma_channel_set_trans_count(channel, 8192, false);
    dma_channel_set_read_addr(channel, buffer, true);

    return true;
}

bool DRAW_isRun(){
#if INTERFACE_UPDATE_VIA_DMA
    return dma_channel_is_busy(channel);
#else
    return false;
#endif
}