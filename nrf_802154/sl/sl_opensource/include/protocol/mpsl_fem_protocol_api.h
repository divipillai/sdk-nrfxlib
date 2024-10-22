/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/**
 * @file mpsl_fem_protocol_api.h
 *
 * @defgroup mpsl_fem MPSL Protocol interface for Power Amplifier (PA) and Low Noise Amplifier (LNA).
 * @ingroup  mpsl
 *
 * This module provides the interface for the protocols to use PA, LNA, or both
 * in the radio transmission and the radio reception. Refer to the README.rst
 * for the details.
 *
 * @{
 */

#ifndef MPSL_FEM_PROTOCOL_API_H__
#define MPSL_FEM_PROTOCOL_API_H__

#include <stdint.h>
#include <stdbool.h>

#include <nrf.h>
#include "nrf_errno.h"

/** @brief PA and LNA functionality types. */
typedef enum
{
    MPSL_FEM_PA  = 1 << 0,                     /**< PA Functionality. */
    MPSL_FEM_LNA = 1 << 1,                     /**< LNA Functionality. */
    MPSL_FEM_ALL = MPSL_FEM_PA | MPSL_FEM_LNA  /**< Both PA and LNA Functionalities. */
} mpsl_fem_functionality_t;

/** @brief PA and LNA activation event types. */
typedef enum
{
    MPSL_FEM_EVENT_TYPE_TIMER,                 /**< Timer Event type. */
    MPSL_FEM_EVENT_TYPE_GENERIC                /**< Generic Event type. */
} mpsl_fem_event_type_t;

/** @brief Type representing a multiple-subscribable hardware event.
 *
 *  For nRF52 series this is an address of an event within a peripheral. This
 *  event can be written to @c EEP register of a PPI channel, to make the
 *  PPI channel be driven by the event. For nRF52 series an event can be
 *  published to multiple PPI channels by hardware design, which makes it possible
 *  for multiple tasks to subscribe to it.
 *
 *  For nRF53 series, this is a number of a DPPI channel which is configured
 *  in such a way that certain event publishes to the DPPI channel and the
 *  DPPI channel is enabled. Ensuring above is responsibility of a user
 *  of the provided API. Multiple tasks can then subscribe to the DPPI channel
 *  (by hardware design), thus indirectly to the event.
 */
#ifdef PPI_PRESENT
typedef uint32_t mpsl_subscribable_hw_event_t;
#else
typedef uint8_t mpsl_subscribable_hw_event_t;
#endif

/** @brief MPSL Front End Module event. */
typedef struct
{
    /** Type of event source. */
    mpsl_fem_event_type_t      type;
    /** Implementation of the event. */
    union
    {
        /** Parameters when type is @ref MPSL_FEM_EVENT_TYPE_TIMER. */
        struct
        {
            /** Pointer to a 1-us resolution timer instance. */
            NRF_TIMER_Type *   p_timer_instance;

            /** Counter period parameters */
            struct
            {
                /** Timer value when the Front End Module can start preparing PA/LNA. */
                uint32_t       start;
                /** Timer value at which the PA/LNA have to be prepared. Radio operation shall start at this point. */
                uint32_t       end;
              /** Time interval in which the timer should start and end. */
            } counter_period;

            /** Mask of the compare channels that can be used by the Front End Module to schedule its own tasks. */
            uint8_t            compare_channel_mask;
          /** Event generated by the timer, used in case of type equal to @ref MPSL_FEM_EVENT_TYPE_TIMER. */
        } timer;

        /** Parameters when type is @ref MPSL_FEM_EVENT_TYPE_GENERIC. */
        struct
        {
            /** Event triggering required FEM operation. */
            mpsl_subscribable_hw_event_t event;
          /** Generic event, used in case of type equal to @ref MPSL_FEM_EVENT_TYPE_GENERIC. */
        } generic;
    } event;

#if defined(NRF52_SERIES)
    /** False to ignore the PPI channel below and use the one set by application. True to use the PPI channel below. */
    bool                       override_ppi;
    /** PPI channel to be used for this event. */
    uint8_t                    ppi_ch_id;
#endif
} mpsl_fem_event_t;

/** TX power, dBm. */
typedef int8_t mpsl_tx_power_t;

/** Type for PA power control to be applied to Front-End Module, depending on its type.
 * 
 *  The meaning of this type is FEM type-specific.
 */
typedef uint8_t mpsl_fem_pa_power_control_t;


/** @brief Represents components of tx_power to be applied for stages on transmit path. */
typedef struct
{
    /** TX power to be applied to the RADIO peripheral. */
    mpsl_tx_power_t radio_tx_power;

    /** FEM PA power control.*/
    mpsl_fem_pa_power_control_t fem_pa_power_control;
} mpsl_tx_power_split_t;

/** @brief PA setup is required before starting a transmission.
 *
 *  This flag applies to @ref mpsl_fem_caps_t::flags.
 * 
 *  If it is set, then @ref mpsl_fem_pa_configuration_set must be called before transmission starts.
 */
#define MPSL_FEM_CAPS_FLAG_PA_SETUP_REQUIRED  (1U << 0)

/** @brief LNA setup is required before starting a reception.
 *
 *  This flag applies to @ref mpsl_fem_caps_t::flags.
 * 
 *  If it is set, then @ref mpsl_fem_lna_configuration_set must be called before reception starts.
 */
#define MPSL_FEM_CAPS_FLAG_LNA_SETUP_REQUIRED (1U << 1)

/** @brief Structure representing capabilities and characteristics of the FEM in use. */
typedef struct
{
    /** Flags informing about the FEM in use.
     *
     *  The following flags apply:
     *  @ref MPSL_FEM_CAPS_FLAG_PA_SETUP_REQUIRED, @ref MPSL_FEM_CAPS_FLAG_LNA_SETUP_REQUIRED
     */
    uint32_t flags;
} mpsl_fem_caps_t;

/** @brief Gets the capabilities of the FEM in use.
 * 
 *  @param[out] p_caps  Pointer to the capabilities structure to be filled in.
 */
void mpsl_fem_caps_get(mpsl_fem_caps_t * p_caps);

/** @brief Enable Front End Module.
 *
 *  Some Front End Module devices should be explicitly enabled before they can be configured
 *  for radio operation. This function is intended to enable Front End Module shortly before
 *  radio operations are started.
 * 
 *  After the Front End Module performed all it's operations complementary @ref mpsl_fem_disable
 *  function should be called.
 */
void mpsl_fem_enable(void);

/** @brief Disable Front End Module.
 *
 * Some Front End Module devices can be explicitly disabled after PA and LNA activities are
 * finished to preserve power.
 *
 * This function is intended to disable Front End Module shortly after radio operations are
 * finished and the protocol does not expect more radio activities in short future, or passes
 * radio control to other protocols in the system.
 *
 * Front End Module disabling process is synchronous and immediate.
 *
 * @retval 0
 * @retval -NRF_EPERM    FEM is configured to enable PA or LNA.
 */
int32_t mpsl_fem_disable(void);

/** @brief Sets up PA using the provided events for the upcoming radio transmission.
 *
 * Multiple configurations can be provided by repeating calls to this function
 * (that is, you can set the activate and the deactivate events in multiple calls,
 * and the configuration is preserved between calls).
 *
 * The order of calls of this function and its `lna` counterpart must match the
 * order of radio operations.
 * I.e. if you want to listen first and then send the frame, you need first to
 * issue @ref mpsl_fem_lna_configuration_set and only after @ref mpsl_fem_pa_configuration_set.
 *
 * If a @ref MPSL_FEM_EVENT_TYPE_TIMER timer event is
 * provided, the PA will be configured to activate or deactivate at the
 * application-configured time gap before the timer instance reaches the given
 * register_value. The time gap is set via the corresponding configuration setter
 * of the selected Front End Module.
 *
 * If a @ref MPSL_FEM_EVENT_TYPE_GENERIC event is provided,
 * the PA will be configured to activate or deactivate when an event occurs.
 *
 * The function sets up the PPIs and the GPIOTE channel to activate PA for the
 * upcoming radio transmission. The PA pin will be active until deactivated,
 * which can happen either by encountering a configured deactivation event or by
 * using @ref mpsl_fem_deactivate_now.
 *
 * @param[in] p_activate_event   Pointer to the activation event structure.
 * @param[in] p_deactivate_event Pointer to the deactivation event structure.
 *
 * @pre To activate PA, the corresponding configuration setter of the selected
 * Front End Module must have been called first.
 *
 * @note If a timer event is provided, the caller of this function is responsible
 * for starting the timer and configuring its shorts.
 * Moreover, the caller is responsible for stopping the timer no earlier than
 * the compare channel of the lowest ID among the provided ones does expire.
 *
 * @note The activation event can be only of type
 * @ref MPSL_FEM_EVENT_TYPE_TIMER.
 * Using other activation event type leads to undefined module behavior.
 *
 * @retval   0             PA activation setup is successful.
 * @retval   -NRF_EPERM    PA is currently disabled.
 * @retval   -NRF_EINVAL   PA activation setup could not be performed due to
 *                         invalid or missing configuration parameters
 *                         in p_activate_event or p_deactivate_event, or both.
 */
int32_t mpsl_fem_pa_configuration_set(const mpsl_fem_event_t * const p_activate_event,
                                      const mpsl_fem_event_t * const p_deactivate_event);

/** @brief Clears up the configuration provided by the @ref mpsl_fem_pa_configuration_set function.
 *
 * @retval   0             PA activation setup purge is successful.
 * @retval   -NRF_EPERM    PA is currently disabled.
 */
int32_t mpsl_fem_pa_configuration_clear(void);

/** @brief Sets up LNA using the provided event for the upcoming radio reception.
 *
 * Multiple configurations can be provided by repeating calls to this function
 * (that is, you can set the activate and the deactivate event in multiple calls,
 * and the configuration is preserved between calls).
 *
 * The order of calls of this function and its `pa` counterpart must match the
 * order of radio operations. I.e. if you want to listen first and then send the
 * frame, you need first to issue @ref mpsl_fem_lna_configuration_set and only
 * after @ref mpsl_fem_pa_configuration_set.
 *
 * If a @ref MPSL_FEM_EVENT_TYPE_TIMER timer event is
 * provided, the LNA will be configured to activate or deactivate at the
 * application-configured time gap before the timer instance reaches the given
 * register_value. The time gap is set via the corresponding configuration setter
 * of the selected Front End Module.
 *
 * If a @ref MPSL_FEM_EVENT_TYPE_GENERIC event is provided,
 * the LNA will be configured to activate or deactivate when an event occurs.
 *
 * The function sets up the PPIs and the GPIOTE channel to activate LNA for the
 * upcoming radio transmission. The LNA pin will be active until deactivated,
 * which can happen either by encountering a configured deactivation event or by
 * using @ref mpsl_fem_deactivate_now.
 *
 * @param[in] p_activate_event   Pointer to the activation event structure.
 * @param[in] p_deactivate_event Pointer to the deactivation event structure.
 *
 * @pre To activate LNA, the corresponding configuration setter of the selected
 * Front End Module must have been called first.
 *
 * @note If a timer event is provided, the caller of this function is responsible
 * for starting the timer and configuring its shorts.
 * Moreover, the caller is responsible for stopping the timer no earlier than
 * the compare channel of the lowest ID among the provided ones does expire.
 *
 * @note The activation event can be only of type
 * @ref MPSL_FEM_EVENT_TYPE_TIMER. Using other activation
 * event type leads to undefined module behavior.
 *
 * @retval   0             LNA activation setup is successful.
 * @retval   -NRF_EPERM    LNA is currently disabled.
 * @retval   -NRF_EINVAL   LNA activation setup could not be performed due to
 *                         invalid or missing configuration parameters
 *                         in p_activate_event or p_deactivate_event, or both.
 */
int32_t mpsl_fem_lna_configuration_set(const mpsl_fem_event_t * const p_activate_event,
                                       const mpsl_fem_event_t * const p_deactivate_event);

/** @brief Clears up the configuration provided by the @ref mpsl_fem_lna_configuration_set function.
 *
 * @retval   0             LNA activate setup purge is successful.
 * @retval   -NRF_EPERM    LNA is currently disabled.
 */
int32_t mpsl_fem_lna_configuration_clear(void);

/** @brief Deactivates PA/LNA with immediate effect.
 *
 *  Deactivates PA/LNA with immediate effect - contrary to
 *  @ref mpsl_fem_lna_configuration_clear or @ref mpsl_fem_pa_configuration_clear,
 *  which both just set up the infrastructure for events which shall disable the PA/LNA.
 *
 * @param[in] type Whether to deactivate imeediately PA, LNA, or both (see @ref mpsl_fem_functionality_t).
 */
void mpsl_fem_deactivate_now(mpsl_fem_functionality_t type);

/** @brief Instruct Front End Module to disable PA and LNA as soon as possible
 *  using the group following the event.
 *
 * @param[in] event       An event which is triggered when the abort condition occurs.
 *                        (See doc for @ref mpsl_subscribable_hw_event_t type.)
 * @param[in] group       (D)PPI Group which shall be disabled when the abort event is triggered.
 *
 * @retval   0            Setting of the abort sequence path is successful.
 * @retval   -NRF_EPERM   Setting of the abort sequence path could not be performed.
 */
int32_t mpsl_fem_abort_set(mpsl_subscribable_hw_event_t event, uint32_t group);

/** @brief Adds one more PPI channel to the PPI Group prepared by the
 *  @ref mpsl_fem_abort_set function.
 *
 * @param[in] channel_to_add (D)PPI channel to add to the (D)PPI group.
 * @param[in] group          The said PPI group.
 *
 * @retval    0              Setting of the abort sequence path is successful.
 * @retval    -NRF_EPERM     Setting of the abort sequence path could not be performed.
 */
int32_t mpsl_fem_abort_extend(uint32_t channel_to_add, uint32_t group);

/** @brief Removes one PPI channel from the PPI Group prepared by the
 *  @ref mpsl_fem_abort_set function.
 *
 * @param[in] channel_to_remove (D)PPI channel to remove from the (D)PPI group.
 * @param[in] group             The said PPI group.
 *
 * @retval   0                  Setting of the abort sequence path is successful.
 * @retval   -NRF_EPERM         Setting of the abort sequence path could not be performed.
 */
int32_t mpsl_fem_abort_reduce(uint32_t channel_to_remove, uint32_t group);

/** @brief Clears up the configuration provided by the @ref mpsl_fem_abort_set
 *  function.
 *
 * @retval   0          Clearing of the abort sequence path is successful.
 * @retval   -NRF_EPERM Clearing was not done - the possible reason is that there was nothing to clear.
 */
int32_t mpsl_fem_abort_clear(void);

/** @brief Cleans up the configured PA/LNA hardware resources.
 *
 *  The function resets the hardware that has been set up for the PA/LNA
 *  activation. The PA and LNA module control configuration parameters
 *  are not deleted.
 *  The function is intended to be called after the radio DISABLED signal.
 */
void mpsl_fem_cleanup(void);

/** @brief Splits transmit power value into components to be applied on each stage on transmit path.
 *
 * @note If the exact value of @p power cannot be achieved, this function attempts to either use
 * available level lower than the requested level to not exceed constraint, or use the lowest
 * available level greater than the requested level, depending on @p tx_power_ceiling.
 *
 * @param[in]  power                TX power requested for transmission on air.
 * @param[out] p_tx_power_split     Components of tx_power to be applied for stages on transmit path.
 *                                  If requested @p power is too high, the split will be set to
 *                                  a value representing maximum achievable power. If the requested
 *                                  @p power is too low, the split will be set to a value representing
 *                                  minimum achievable power.
 * @param[in]  freq_mhz             Frequency in MHz to calculate the split for.
 * @param[in]  tx_power_ceiling     Flag to get ceiling or floor of requested transmit power level.
 *
 * @return  The power in dBm that will be achieved if values returned through @p p_tx_power_split
 *          are applied.
 */
int8_t mpsl_fem_tx_power_split(const mpsl_tx_power_t         power,
                               mpsl_tx_power_split_t * const p_tx_power_split,
                               uint16_t                      freq_mhz,
                               bool                          tx_power_ceiling);

/** @brief Sets the PA power control.
 * 
 * Setting the PA power control informs the FEM implementation how the PA is to be controlled
 * before the next transmission.
 * 
 * The PA power control set by this function is to be applied to control signals or
 * parameters. What signals and parameters are controlled and how does it happen depends on
 * implementation of given FEM. The meaning of @p pa_power_control parameter is
 * fully FEM type-dependent. For FEM type-independent protocol implementation please
 * use the function @ref mpsl_fem_tx_power_split and provide outcome of this function
 * returned by the parameter @c p_tx_power_split to the call to @ref mpsl_fem_pa_power_control_set.
 * For applications intended for testing the FEM itself when @ref mpsl_fem_tx_power_split is not used
 * you must make the @p pa_power_control parameter on your own.
 *
 * @note The PA power control set by this function will be applied to radio transmissions
 * following the call. If the function is called during radio transmission
 * or during ramp-up for transmission it is unspecified if the control is applied.
 *
 * @param[in] pa_power_control  PA power control to be applied to the FEM.
 *
 * @retval   0             PA power control has been applied successfully.
 * @retval   -NRF_EINVAL   PA power control could not be applied. Provided @p pa_power_control is invalid.
 */
int32_t mpsl_fem_pa_power_control_set(mpsl_fem_pa_power_control_t pa_power_control);

/** @brief Prepares the Front End Module to switch to the Power Down state.
 *
 *  @deprecated This function is deprecated. Use @ref mpsl_fem_disable instead.
 *
 *  This function makes sure the Front End Module shall be switched off in the
 *  appropriate time, using the hardware timer and its compare channel.
 *  The timer is owned by the protocol and must be started by the protocol.
 *  The timer stops after matching the provided compare channel (the call sets the short).
 *
 * @param[in] p_instance      Timer instance that is used to schedule the transition to the Power Down state.
 * @param[in] compare_channel Compare channel to hold a value for the timer.
 * @param[in] ppi_id          ID of the PPI channel used to switch to the Power Down state.
 * @param[in] event_addr      Address of the event which shall trigger the Timer start.
 *
 * @retval true               Whether the scheduling of the transition was successful.
 * @retval false              Whether the scheduling of the transition was not successful.
 */
bool mpsl_fem_prepare_powerdown(NRF_TIMER_Type * p_instance,
                                uint32_t         compare_channel,
                                uint32_t         ppi_id,
                                uint32_t         event_addr);

#endif // MPSL_FEM_PROTOCOL_API_H__

/**@} */
