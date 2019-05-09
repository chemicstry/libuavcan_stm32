/*
 * Copyright (C) 2014 Pavel Kirienko <pavel.kirienko@gmail.com>
 */

#pragma once

#include <uavcan_stm32/build_config.hpp>
#include <uavcan/driver/system_clock.hpp>

#include <ch.h>

namespace uavcan_stm32
{

namespace clock
{
/**
 * Starts the clock.
 * Can be called multiple times, only the first call will be effective.
 */
void init();

/**
 * Returns current monotonic time since the moment when clock::init() was called.
 * This function is thread safe.
 */
uavcan::uint64_t sampleMonotonicFromCriticalSection();
uavcan::MonotonicTime getMonotonic();

/**
 * Sets the driver's notion of the system UTC. It should be called
 * at startup and any time the system clock is updated from an
 * external source that is not the UAVCAN Timesync master.
 * This function is thread safe.
 */
void setUtc(uavcan::UtcTime time);

/**
 * Returns UTC time if it has been set, otherwise returns zero time.
 * This function is thread safe.
 */
uavcan::uint64_t sampleUtcFromCriticalSection();
uavcan::UtcTime getUtc();

/**
 * Performs UTC phase and frequency adjustment.
 * The UTC time will be zero until first adjustment has been performed.
 * This function is thread safe.
 */
void adjustUtc(uavcan::UtcDuration adjustment);

/**
 * UTC clock synchronization parameters
 */
struct UtcSyncParams
{
    float offset_p;                        ///< PPM per one usec error
    float rate_i;                          ///< PPM per one PPM error for second
    float rate_error_corner_freq;
    float max_rate_correction_ppm;
    float lock_thres_rate_ppm;
    uavcan::UtcDuration lock_thres_offset;
    uavcan::UtcDuration min_jump;          ///< Min error to jump rather than change rate

    UtcSyncParams()
        : offset_p(0.01F)
        , rate_i(0.02F)
        , rate_error_corner_freq(0.01F)
        , max_rate_correction_ppm(300.0F)
        , lock_thres_rate_ppm(2.0F)
        , lock_thres_offset(uavcan::UtcDuration::fromMSec(4))
        , min_jump(uavcan::UtcDuration::fromMSec(10))
    { }
};

/**
 * Clock rate error.
 * Positive if the hardware timer is slower than reference time.
 * This function is thread safe.
 */
float getUtcRateCorrectionPPM();

/**
 * Number of non-gradual adjustments performed so far.
 * Ideally should be zero.
 * This function is thread safe.
 */
uavcan::uint32_t getUtcJumpCount();

/**
 * Whether UTC is synchronized and locked.
 * This function is thread safe.
 */
bool isUtcLocked();

/**
 * Returns usec error between time master and internal utc clock
 * This function is thread safe.
 */
uavcan::int64_t getUtcSyncError();

/**
 * UTC sync params get/set.
 * Both functions are thread safe.
 */
UtcSyncParams getUtcSyncParams();
void setUtcSyncParams(const UtcSyncParams& params);

/**
 * Sets the precise UTC time for the next PPS. Used for time sync.
 * This function is thread safe.
 */
void setUtcNextPPS(uavcan::uint64_t time);

enum ExternalEventChannels : uint8_t
{
    EXT_EVENT_NONE      = 0,
    EXT_EVENT_CH1       = 1 << 0,
    EXT_EVENT_CH2       = 1 << 1,
    EXT_EVENT_CH3       = 1 << 2,
    EXT_EVENT_CH4       = 1 << 3,
    EXT_EVENT_COUNT     = 4,
};

inline ExternalEventChannels operator|(ExternalEventChannels lhs, ExternalEventChannels rhs) {
    return (ExternalEventChannels)((int)lhs|(int)rhs);
}

struct ExternalEvent
{
    uint64_t utc;
    uint8_t channel;
    uint32_t id;
};

void setExternalEventChannels(ExternalEventChannels channels);
bool fetchExternalEvent(ExternalEvent*evt, sysinterval_t timeout);

}

/**
 * Adapter for uavcan::ISystemClock.
 */
class SystemClock : public uavcan::ISystemClock, uavcan::Noncopyable
{
    SystemClock() { }

    virtual void adjustUtc(uavcan::UtcDuration adjustment) { clock::adjustUtc(adjustment); }

public:
    virtual uavcan::MonotonicTime getMonotonic() const { return clock::getMonotonic(); }
    virtual uavcan::UtcTime getUtc()             const { return clock::getUtc(); }

    /**
     * Calls clock::init() as needed.
     * This function is thread safe.
     */
    static SystemClock& instance();
};

}
