#include "qspi.hpp"

/**
 * @brief Check for Error
 *
 * @return qspi_driver::error_t
 */
qspi_driver::error_t qspi_driver::check_error()
{
    uint32_t error = (_ptr->SR & (QUADSPI_SR_TEF | QUADSPI_SR_TOF));
    _ptr->FCR = (QUADSPI_FCR_CTEF | QUADSPI_FCR_CTOF);
    if (error & QUADSPI_SR_TEF)
    {
        return QSPI_HARDWARE_ERROR;
    }
    if (error & QUADSPI_SR_TOF)
    {
        return QSPI_TIME_OUT;
    }
    return QSPI_OK;
}
/**
 * @brief set the commands for transmission
 *
 * @param header
 * @param mode
 */
void qspi_driver::set_header(const qspi_driver::header_t &header, qspi_driver::cmd_data_mode data_mode, fmode mode)
{
    uint32_t ccr = (static_cast<uint32_t>(header.instruction.mode) << QUADSPI_CCR_IMODE_Pos) |
                   (static_cast<uint32_t>(header.instruction.cmd) << QUADSPI_CCR_INSTRUCTION_Pos) |
                   (static_cast<uint32_t>(header.address.mode) << QUADSPI_CCR_ADMODE_Pos) |
                   (static_cast<uint32_t>(header.address.size) << QUADSPI_CCR_ADSIZE_Pos) |
                   (static_cast<uint32_t>(header.alternative_byte.mode) << QUADSPI_CCR_ABMODE_Pos) |
                   (static_cast<uint32_t>(header.alternative_byte.size) << QUADSPI_CCR_ABSIZE_Pos) |
                   (static_cast<uint32_t>(header.dummy_cycles) << QUADSPI_CCR_DCYC_Pos) |
                   (static_cast<uint32_t>(mode) << QUADSPI_CCR_FMODE_Pos) |
                   (static_cast<uint32_t>(data_mode) << QUADSPI_CCR_DMODE_Pos) |
                   (static_cast<uint32_t>(header.sio0) << QUADSPI_CCR_SIOO_Pos) |
                   (static_cast<uint32_t>(header.ddr.delay) << QUADSPI_CCR_DHHC_Pos) |
                   (static_cast<uint32_t>(header.ddr.mode) << QUADSPI_CCR_DDRM_Pos);

    _ptr->CCR = ccr;
}

/**
 * @brief poll for the bits
 *
 * @param poll
 * @return qspi_driver::error_t
 */
qspi_driver::error_t qspi_driver::poll(const qspi_driver::polling_t &poll)
{
    while (_ptr->SR & QUADSPI_SR_BUSY)
    {
    }
    if (poll.poll.size > 0)
    {
        _ptr->DLR = poll.poll.size - 1;
    }

    _ptr->CR = (_ptr->CR & ~(QUADSPI_CR_PMM | QUADSPI_CR_APMS)) |
               (static_cast<uint32_t>(poll.poll.match_mode) << QUADSPI_CR_PMM_Pos) |
               (static_cast<uint32_t>(poll.poll.autostop) << QUADSPI_CR_APMS_Pos);
    _ptr->ABR = poll.header.alternative_byte.alternate_bytes;
    _ptr->PSMAR = poll.poll.match;
    _ptr->PSMKR = poll.poll.mask;
    _ptr->PIR = static_cast<uint32_t>(poll.poll.interval);
    set_header(poll.header, poll.poll.mode, AUTO_POLL);
    _ptr->AR = poll.header.address.address;
    while ((_ptr->SR & QUADSPI_SR_SMF) == 0)
    {
        auto res = check_error();
        if (res != QSPI_OK)
        {
            return res;
        }
    }
    return QSPI_OK;
}

/**
 * @brief
 *
 * @param transaction
 * @return qspi_driver::error_t
 */
qspi_driver::error_t qspi_driver::write(const qspi_driver::transact_t &transaction)
{
    while (_ptr->SR & QUADSPI_SR_BUSY)
    {
    }
    // Instruction phase
    if (transaction.data.size > 0)
    {
        _ptr->DLR = transaction.data.size - 1;
    }
    set_header(transaction.header, transaction.data.mode, INDIRECT_WRITE);
    _ptr->ABR = transaction.header.alternative_byte.alternate_bytes;
    // Address phase
    _ptr->AR = transaction.header.address.address;
    // Data phase
    if (transaction.data.size > 0)
    {
        for (uint32_t i = 0; i < transaction.data.size; i++)
        {
            while ((_ptr->SR & QUADSPI_SR_FTF) == 0)
            {
                auto res = check_error();
                if (res != QSPI_OK)
                {
                    return res;
                }
            }
            *((__IO uint8_t *)&_ptr->DR) = transaction.data.buf[i];
        }
    }
    while ((_ptr->SR & QUADSPI_SR_TCF) == 0)
    {
        auto res = check_error();
        if (res != QSPI_OK)
        {
            return res;
        }
    };
    return QSPI_OK;
}

/**
 * @brief transferring read
 *
 * @param transaction
 * @return qspi_driver::error_t QSPI_OK if successfull, error otherwise
 */
qspi_driver::error_t qspi_driver::read(qspi_driver::transact_t &transaction)
{
    while (_ptr->SR & QUADSPI_SR_BUSY)
    {
    }
    if (transaction.data.size > 0)
    {
        _ptr->DLR = transaction.data.size - 1;
    }
    // Instruction phase
    set_header(transaction.header, transaction.data.mode, INDIRECT_READ);

    _ptr->ABR = transaction.header.alternative_byte.alternate_bytes;
    // Address phase
    _ptr->AR = transaction.header.address.address;
    // Data phase

    if (transaction.data.size > 0)
    {
        for (uint32_t i = 0; i < transaction.data.size; i++)
        {
            // Poll FTF and TCF flag
            while ((_ptr->SR & (QUADSPI_SR_TCF | QUADSPI_SR_FTF)) == 0)
            {
                auto res = check_error();
                if (res != QSPI_OK)
                {
                    return res;
                }
            }
            uint8_t val = *((__IO uint8_t *)&_ptr->DR);
            transaction.data.buf[i] = val;
        }
    }

    while ((_ptr->SR & QUADSPI_SR_TCF) == 0)
    {
        auto res = check_error();
        if (res != QSPI_OK)
        {
            return res;
        }
    };
    return QSPI_OK;
}

qspi_driver::error_t qspi_driver::abort()
{
    _ptr->CR |= QUADSPI_CR_ABORT;
    while ((_ptr->SR & (QUADSPI_SR_BUSY | QUADSPI_SR_TCF)) != 0)
    {
        auto res = check_error();
        if (res != QSPI_OK)
        {
            return res;
        }
    }
    return QSPI_OK;
}

qspi_driver::error_t qspi_driver::mmap(const qspi_driver::memmap_t &transaction)
{
    while (_ptr->SR & QUADSPI_SR_BUSY)
    {
    }
    _ptr->CR = (_ptr->CR & ~(QUADSPI_CR_TCEN)) |
               (static_cast<uint32_t>(transaction.memmap.is_timeout) << QUADSPI_CR_TCEN_Pos);
    _ptr->LPTR = transaction.memmap.period;
    set_header(transaction.header, transaction.memmap.mode, MEM_MAP);
    return QSPI_OK;
}