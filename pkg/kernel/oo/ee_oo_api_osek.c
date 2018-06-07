/* ###*B*###
 * Erika Enterprise, version 3
 * 
 * Copyright (C) 2017 Evidence s.r.l.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License, version 2, for more details.
 * 
 * You should have received a copy of the GNU General Public License,
 * version 2, along with this program; if not, see
 * <https://www.gnu.org/licenses/old-licenses/gpl-2.0.html >.
 * 
 * This program is distributed to you subject to the following
 * clarifications and special exceptions to the GNU General Public
 * License, version 2.
 * 
 * THIRD PARTIES' MATERIALS
 * 
 * Certain materials included in this library are provided by third
 * parties under licenses other than the GNU General Public License. You
 * may only use, copy, link to, modify and redistribute this library
 * following the terms of license indicated below for third parties'
 * materials.
 * 
 * In case you make modified versions of this library which still include
 * said third parties' materials, you are obligated to grant this special
 * exception.
 * 
 * The complete list of Third party materials allowed with ERIKA
 * Enterprise version 3, together with the terms and conditions of each
 * license, is present in the file THIRDPARTY.TXT in the root of the
 * project.
 * ###*E*### */

/** \file  ee_oo_api_osek.c
 *  \brief  OSEK Kernel APIs Implementation.
 *
 *  This files contains the implementation of all OSEK Kernel APIs in
 *  Erika Enterprise.
 *
 *  \note  TO BE DOCUMENTED!!!
 *
 *  \author  Errico Guidieri
 *  \date  2016
 */
#include "ee_internal.h"

/* [SWS_Os_00299] The Operating System module shall provide the services
   DisableAllInterrupts(), EnableAllInterrupts(), SuspendAllInterrupts(),
   ResumeAllInterrupts() prior to calling StartOS() and after calling
   ShutdownOS(). (SRS_Os_11018) */
FUNC(void, OS_CODE)
  DisableAllInterrupts
(
  void
)
{
  CONSTP2VAR(OsEE_CDB, AUTOMATIC, OS_APPL_DATA) p_cdb = osEE_get_curr_core();
  CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA) p_ccb = p_cdb->p_ccb;
  /* Disable Immediately for Atomicity */
  osEE_hal_disableIRQ();
  osEE_orti_trace_service_entry(p_ccb, OSServiceId_DisableAllInterrupts);
  p_ccb->d_isr_all_cnt = 1U;
  osEE_orti_trace_service_exit(p_ccb, OSServiceId_DisableAllInterrupts);
  return;
}

FUNC(void, OS_CODE)
  EnableAllInterrupts
(
  void
)
{
  /* [SWS_Os_00092] If EnableAllInterrupts()/ResumeAllInterrupts()/
   * ResumeOSInterrupts() are called and no corresponding DisableAllInterupts()
   * /SuspendAllInterrupts()/SuspendOSInterrupts() was done before, the
   * Operating System module shall not perform this Operating System service.
   * (SRS_Os_11009) */
  CONSTP2VAR(OsEE_CDB, AUTOMATIC, OS_APPL_DATA) p_cdb = osEE_get_curr_core();
  CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA) p_ccb = p_cdb->p_ccb;

  osEE_orti_trace_service_entry(p_ccb, OSServiceId_EnableAllInterrupts);

  if (p_ccb->d_isr_all_cnt > 0U) {
    p_ccb->d_isr_all_cnt = 0U;
    osEE_hal_enableIRQ();
  }

  osEE_orti_trace_service_exit(p_ccb, OSServiceId_EnableAllInterrupts);

  return;
}

FUNC(void, OS_CODE)
  SuspendAllInterrupts
(
  void
)
{
  CONSTP2VAR(OsEE_CDB, AUTOMATIC, OS_APPL_DATA) p_cdb = osEE_get_curr_core();
  CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA) p_ccb = p_cdb->p_ccb;

  osEE_orti_trace_service_entry(p_ccb, OSServiceId_SuspendAllInterrupts);

  if (p_ccb->s_isr_all_cnt == 0U) {
    CONST(OsEE_reg, AUTOMATIC) flags = osEE_hal_suspendIRQ();
    p_ccb->prev_s_isr_all_status = flags;
    ++p_ccb->s_isr_all_cnt;
  } else if (p_ccb->s_isr_all_cnt < OSEE_MAX_BYTE) {
    ++p_ccb->s_isr_all_cnt;
  }

  osEE_orti_trace_service_exit(p_ccb, OSServiceId_SuspendAllInterrupts);

  return;
}

FUNC(void, OS_CODE)
  ResumeAllInterrupts
(
  void
)
{
  CONSTP2VAR(OsEE_CDB, AUTOMATIC, OS_APPL_DATA) p_cdb = osEE_get_curr_core();
  CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA) p_ccb = p_cdb->p_ccb;

  osEE_orti_trace_service_entry(p_ccb, OSServiceId_ResumeAllInterrupts);

  if (p_ccb->s_isr_all_cnt > 0U) {
    if (--p_ccb->s_isr_all_cnt == 0U) {
      osEE_hal_resumeIRQ(p_ccb->prev_s_isr_all_status);
    }
  }

  osEE_orti_trace_service_exit(p_ccb, OSServiceId_ResumeAllInterrupts);

  return;
}

FUNC(void, OS_CODE)
  SuspendOSInterrupts
(
  void
)
{
  CONSTP2VAR(OsEE_CDB, AUTOMATIC, OS_APPL_DATA) p_cdb = osEE_get_curr_core();
  CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA) p_ccb = p_cdb->p_ccb;

  osEE_orti_trace_service_entry(p_ccb, OSServiceId_SuspendOSInterrupts);

  if (p_ccb->s_isr_os_cnt == 0U) {
    CONST(OsEE_reg, AUTOMATIC) flags = osEE_hal_begin_nested_primitive();
    p_ccb->prev_s_isr_os_status = flags;
    ++p_ccb->s_isr_os_cnt;
  } else if (p_ccb->s_isr_os_cnt < OSEE_MAX_BYTE) {
    ++p_ccb->s_isr_os_cnt;
  }

  osEE_orti_trace_service_exit(p_ccb, OSServiceId_SuspendOSInterrupts);

  return;
}

FUNC(void, OS_CODE)
  ResumeOSInterrupts
(
  void
)
{
  CONSTP2VAR(OsEE_CDB, AUTOMATIC, OS_APPL_DATA) p_cdb = osEE_get_curr_core();
  CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA) p_ccb = p_cdb->p_ccb;

  osEE_orti_trace_service_entry(p_ccb, OSServiceId_ResumeOSInterrupts);

  if (p_ccb->s_isr_os_cnt > 0U) {
    if (--p_ccb->s_isr_os_cnt == 0U) {
      osEE_hal_end_nested_primitive(p_ccb->prev_s_isr_os_status);
    }
  }

  osEE_orti_trace_service_exit(p_ccb, OSServiceId_ResumeOSInterrupts);

  return;
}

FUNC(StatusType, OS_CODE)
  StartOS
(
  VAR(AppModeType, AUTOMATIC) Mode
)
{
  VAR(StatusType, AUTOMATIC)                       ev;
  VAR(AppModeType, AUTOMATIC)               real_mode = Mode;
#if (!defined(OSEE_SINGLECORE))
  CONST(CoreIdType, AUTOMATIC)           curr_core_id = osEE_get_curr_core_id();
  CONSTP2VAR(OsEE_KDB, AUTOMATIC, OS_APPL_DATA) p_kdb = osEE_get_kernel();
  CONSTP2VAR(OsEE_KCB, AUTOMATIC, OS_APPL_DATA) p_kcb = p_kdb->p_kcb;
#endif /* !OSEE_SINGLECORE */
  CONSTP2VAR(OsEE_CDB, AUTOMATIC, OS_APPL_DATA) p_cdb = osEE_get_curr_core();
  CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA) p_ccb = p_cdb->p_ccb;
  CONST(OsEE_reg, AUTOMATIC) flags = osEE_begin_primitive();

  osEE_orti_trace_service_entry(p_ccb, OSServiceId_StartOS);
#if (defined(OSEE_ALLOW_TASK_MIGRATION))
  osEE_lock_kernel();
#endif /* OSEE_ALLOW_TASK_MIGRATION */

  if (p_ccb->os_status != OSEE_KERNEL_INITIALIZED) {
#if (defined(OSEE_ALLOW_TASK_MIGRATION))
    osEE_unlock_kernel();
#endif /* OSEE_ALLOW_TASK_MIGRATION */
    ev = E_OS_ACCESS;
  } else
  if (
#if (!defined(OSEE_SINGLECORE))
    (curr_core_id == OS_CORE_ID_MASTER) &&
    /* I rely in C shortcut for boolean expression */
#endif /* !OSEE_SINGLECORE */
    !osEE_cpu_startos()
  )
  {
#if (defined(OSEE_ALLOW_TASK_MIGRATION))
    osEE_unlock_kernel();
#endif /* OSEE_ALLOW_TASK_MIGRATION */
    ev = E_OS_SYS_INIT;
  } else {
#if (!defined(OSEE_STARTOS_RETURN))
    CONSTP2VAR(OsEE_TDB, AUTOMATIC, OS_APPL_DATA)
      p_idle_tdb = p_cdb->p_idle_task;
#endif /* !OSEE_STARTOS_RETURN */

#if (defined(OSEE_API_DYNAMIC))
    CONSTP2VAR(OsEE_TCB, AUTOMATIC, OS_APPL_DATA)
      p_idle_tcb = p_idle_tdb->p_tcb;
      /* Set Idle TASK as RUNNING */

    /* Fill TCB */
    p_idle_tcb->status              = OSEE_TASK_RUNNING;
    p_idle_tcb->current_num_of_act  = 1U;
#if (defined(OSEE_ALLOW_TASK_MIGRATION))
    p_idle_tcb->current_core_id     = osEE_get_curr_core_id();
#endif /* OSEE_ALLOW_TASK_MIGRATION */

    /* Fill CCB */
    p_ccb->p_curr                   = p_idle_tdb;
#endif /* OSEE_API_DYNAMIC */

    /* GetActiveApplicationMode can be called inside StartupHook */
    p_ccb->os_status                = OSEE_KERNEL_STARTING;
    p_ccb->app_mode                 = real_mode;

    /* Multicore Startup */
#if (!defined(OSEE_SINGLECORE))
/* [SWS_Os_00609] If StartOS is called with the AppMode "DONOTCARE" the
    application mode of the other core(s) (differing from "DONOTCARE")
    shall be used. (SRS_Os_80006) */
/* [SWS_Os_00610] At least one core shall define an AppMode other than
     "DONOTCARE". (SRS_Os_80006) */
/* [SWS_Os_00611] If the IOC is configured, StartOS shall initialize the data
     structures of the IOC. (SRS_Os_80020) */
    if (!(p_kcb->ar_core_mask & ((CoreMaskType)1U << curr_core_id))) {
      while (1) {
        ; /* Endless Loop */
      }
    }
/* [SWS_Os_00580] All cores that belong to the AUTOSAR system shall be
    synchronized within the StartOS before the global StartupHook is called.
    (SRS_Os_80006) */
/* [SWS_Os_00581] The global StartupHook shall be called on all cores
    immediately after the first synchronization point. (SRS_Os_80006) */
      /* Synchronize Cores in Startup. Extracted from AUSTOSAR 4.3.1
          Specification paragraph 7.9.4 Multi-Core start-up concept:
          "This release of the AUTOSAR specification does not support timeouts
          during the synchronization phase. Cores that are activated with
          StartCore but do not call StartOS may cause the system to hang.
          It is in the responsibility of the integrator to avoid such
          behavior." */
    osEE_hal_sync_barrier(p_kdb->p_barrier, &p_kcb->ar_core_mask);

    /* Initialize Slaves Hardware after First synchronization point:
       This assure that all the Master Initializations have been done. */
    /* I rely in C shortcut for boolean expression */
    if ((curr_core_id != OS_CORE_ID_MASTER) && !osEE_cpu_startos()) {
      /* Enter in an endless loop if it happened */
      while (1) {
        ; 
      }
    }
/* [SWS_Os_00608] If more than one core calls StartOS with an AppMode other
    than "DONOTCARE", the AppModes shall be the same. StartOS shall check this
    at the first synchronization point. In case of violation,
    StartOS shall not start the scheduling, shall not call any StartupHooks,
    and shall enter an endless loop on every core. (SRS_Os_80006) */
    {
      VAR(CoreIdType, AUTOMATIC)  i;

      for (i = 0U; i < OSEE_CORE_ID_MAX; ++i) {
        CONSTP2VAR(OsEE_CDB, AUTOMATIC, OS_APPL_DATA)
          p_local_cdb = osEE_get_core(i);
        if (p_local_cdb != NULL) {
          AppModeType current_mode = p_local_cdb->p_ccb->app_mode;

          if (current_mode != DONOTCARE) {
            if (real_mode == DONOTCARE) {
              real_mode = current_mode;
            } else if (real_mode != current_mode) {
            /* Error condition specified by SWS_Os_00608 requirement: enter
                in an endless loop */
              while (1) {
                ; 
              }
            } else {
              /* Empty else statement to comply with MISRA 14.10 */
            }
          }
        }
      }

      if (real_mode != DONOTCARE) {
        /* Set mode for this core as real_mode */
        if (Mode == DONOTCARE) {
          p_ccb->app_mode = real_mode;
        }
      } else {
        /* XXX: It is not specified how to handle the condition that no cores
                defines an AppMode different from DONOTCARE.
                I choose to handle it using OSDEFAULTAPPMODE */
        p_ccb->app_mode = OSDEFAULTAPPMODE;
        real_mode = OSDEFAULTAPPMODE;
      }
    }
#endif /* OSEE_SINGLECORE */

    osEE_call_startup_hook(p_ccb);

#if (defined(OSEE_AS_OSAPPLICATIONS))
/* [SWS_Os_00582] The OS-Application-specific StartupHooks shall be called
    after the global StartupHook but only on the cores to which the
    OS-Application is bound. (SRS_Os_80006, SRS_Os_80008) */
/* TODO: Implement this when OS-Applications will be implemented */
#endif /* OSEE_AS_OSAPPLICATIONS */

#if (defined(OSEE_HAS_AUTOSTART_TRIGGER))
    {
      VAR(MemSize, AUTOMATIC) i;
      CONSTP2VAR(OsEE_autostart_trigger, AUTOMATIC, OS_APPL_DATA)
        p_auto_triggers  = &(*p_cdb->p_autostart_trigger_array)[real_mode];

      for (i = 0U; i < p_auto_triggers->trigger_array_size; ++i) {
        CONSTP2VAR(OsEE_autostart_trigger_info, AUTOMATIC, OS_APPL_DATA)
          p_trigger_to_act_info = &(*p_auto_triggers->p_trigger_ptr_array)[i];
        CONSTP2VAR(OsEE_TriggerDB, AUTOMATIC, OS_APPL_DATA)
          p_trigger_to_act_db   = p_trigger_to_act_info->p_trigger_db;

        /* TODO: Handle trigger as alarm or as schedule table */
        (void)osEE_alarm_set_rel(
          p_trigger_to_act_db->p_counter_db,
          p_trigger_to_act_db,
          p_trigger_to_act_info->increment,
          p_trigger_to_act_info->cycle
        );
      }
    }
#endif /* OSEE_HAS_AUTOSTART_TRIGGER */

#if (defined(OSEE_HAS_AUTOSTART_TASK))
    {
      VAR(MemSize, AUTOMATIC) i;
      CONSTP2VAR(OsEE_autostart_tdb, AUTOMATIC, OS_APPL_DATA)
        p_auto_tdb  = &(*p_cdb->p_autostart_tdb_array)[real_mode];
#if (!defined(OSEE_ALLOW_TASK_MIGRATION))
      CONSTP2VAR(OsEE_RQ, AUTOMATIC, OS_APPL_DATA)
        p_rq        = &p_ccb->rq;
      CONSTP2VAR(OsEE_SN *, AUTOMATIC, OS_APPL_DATA)
        pp_free_sn  = &p_ccb->p_free_sn;

      osEE_lock_core(p_cdb);
#else
      CONSTP2VAR(OsEE_RQ, AUTOMATIC, OS_APPL_DATA)
        p_rq        = &p_kcb->rq;
      CONSTP2VAR(OsEE_SN *, AUTOMATIC, OS_APPL_DATA)
        pp_free_sn  = &p_ccb->p_free_sn;
#endif /* !OSEE_ALLOW_TASK_MIGRATION */

      for (i = 0U; i < p_auto_tdb->tdb_array_size; ++i) {
        CONSTP2VAR(OsEE_TDB, AUTOMATIC, OS_APPL_DATA)
          p_tdb_to_act = (*p_auto_tdb->p_tdb_ptr_array)[i];
        CONSTP2VAR(OsEE_TCB, AUTOMATIC, OS_APPL_DATA)
          p_tcb_to_act = p_tdb_to_act->p_tcb;

        /* Mark the autostart-TASK as Activated */
        ++p_tcb_to_act->current_num_of_act;
        p_tcb_to_act->status = OSEE_TASK_READY;

        (void)osEE_scheduler_rq_insert(
          p_rq,
          osEE_sn_alloc(pp_free_sn),
          p_tdb_to_act
        );
      }

#if (!defined(OSEE_ALLOW_TASK_MIGRATION))
      osEE_unlock_core(p_cdb);
#endif /* !OSEE_ALLOW_TASK_MIGRATION */
    }
#endif /* OSEE_HAS_AUTOSTART_TASK */

#if (!defined(OSEE_SINGLECORE))
/* [SWS_Os_00579] All cores that belong to the AUTOSAR system shall be
    synchronized within the StartOS function before the scheduling is started
    and after the global StartupHook is called. (SRS_Os_80001, SRS_Os_80006) */
/* Synchronize Cores in Startup. Extracted from AUSTOSAR 4.3.1
    Specification paragraph 7.9.4 Multi-Core start-up concept:
    "This release of the AUTOSAR specification does not support timeouts
    during the synchronization phase. Cores that are activated with
    StartCore but do not call StartOS may cause the system to hang.
    It is in the responsibility of the integrator to avoid such behavior." */
    osEE_hal_sync_barrier(p_kdb->p_barrier, &p_kcb->ar_core_mask);
#endif /* OSEE_SINGLECORE */

/* [SWS_Os_00607] StartOS shall start the OS on the core on which it is called.
    (SRS_Os_80006, SRS_Os_80013) */
    if (p_ccb->os_status == OSEE_KERNEL_STARTING) {
      p_ccb->os_status = OSEE_KERNEL_STARTED;
    }
#if (defined(OSEE_ALLOW_TASK_MIGRATION))
    osEE_unlock_kernel();
#endif /* OSEE_ALLOW_TASK_MIGRATION */

    osEE_orti_trace_service_exit(p_ccb, OSServiceId_StartOS);

#if (!defined(OSEE_STARTOS_RETURN)) && (!defined(OSEE_API_DYNAMIC))
    if (p_ccb->os_status == OSEE_KERNEL_STARTED) {
      osEE_idle_task_start(p_idle_tdb);
    }
#if (!defined(OSEE_SHUTDOWN_DO_NOT_RETURN_ON_MAIN))
    osEE_hal_disableIRQ();
    osEE_call_shutdown_hook(p_ccb, p_ccb->last_error);
    while (1) {
      ; /* Endless Loop */
    }
#endif /* !OSEE_SHUTDOWN_DO_NOT_RETURN_ON_MAIN */
#elif (defined(OSEE_API_DYNAMIC))
    if (p_ccb->os_status == OSEE_KERNEL_STARTED) {
      if (p_cdb->p_idle_hook != NULL) {
        osEE_idle_task_start(p_idle_tdb);
      } /* No Autostart TASKs with dynamic API */
    }
#elif (defined(OSEE_HAS_AUTOSTART_TASK))
    if (p_ccb->os_status == OSEE_KERNEL_STARTED) {
      /* Schedule Here */
      (void)osEE_scheduler_task_preemption_point(osEE_get_kernel(), p_cdb);
    }
#endif /* !OSEE_STARTOS_RETURN && !OSEE_API_DYNAMIC && !OSEE_HAS_AUTOSTART_TASK */

    ev = E_OK;

    if (p_ccb->os_status == OSEE_KERNEL_STARTED) {
      /* Set-up IPL at Unmasked value in addition to IRQ enabling */
      /* We always use abstract priorities. 0 is the lowest virtual priority,
         this means ISR2 unmasked, for sure. */
      osEE_hal_set_ipl(0U);
      /* OS started correctly: Enable IRQ */
      osEE_hal_enableIRQ();
    }
  }

  if (ev != E_OK) {
    osEE_set_service_id(p_ccb, OSServiceId_StartOS);
    osEE_call_error_hook(p_ccb, ev);
    osEE_orti_trace_service_exit(p_ccb, OSServiceId_StartOS);
    osEE_end_primitive(flags);
  }

  return ev;
}

FUNC(AppModeType, OS_CODE)
  GetActiveApplicationMode
(
  void
)
{
  VAR(AppModeType, AUTOMATIC) app_mode;
  CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA)
    p_ccb = osEE_get_curr_core()->p_ccb;

  osEE_orti_trace_service_entry(p_ccb, OSServiceId_GetActiveApplicationMode);

  if (p_ccb->os_status >= OSEE_KERNEL_STARTING) {
    app_mode = p_ccb->app_mode;
  } else {
    app_mode = INVALID_APPMODE;
  }

  osEE_orti_trace_service_exit(p_ccb, OSServiceId_GetActiveApplicationMode);

  return app_mode;
}

FUNC(StatusType, OS_CODE)
  ActivateTask
(
  VAR(TaskType, AUTOMATIC) TaskID
)
{
  VAR(StatusType, AUTOMATIC)                    ev;
  CONSTP2VAR(OsEE_KDB, AUTOMATIC, OS_APPL_DATA) p_kdb = osEE_get_kernel();
#if (defined(OSEE_HAS_ORTI))
  CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA)
    p_ccb = osEE_get_curr_core()->p_ccb;
  osEE_orti_trace_service_entry(p_ccb, OSServiceId_ActivateTask);
#endif /* OSEE_HAS_ORTI */

  if (!osEE_is_valid_tid(p_kdb, TaskID)) {
    ev = E_OS_ID;
  } else
  {
    CONSTP2VAR(OsEE_TDB, AUTOMATIC, OS_APPL_DATA)
      p_tdb_act = (*p_kdb->p_tdb_ptr_array)[TaskID];

    if (p_tdb_act->task_type <= OSEE_TASK_TYPE_EXTENDED) {
      CONST(OsEE_reg, AUTOMATIC)  flags = osEE_begin_primitive();

      ev = osEE_task_activated(p_tdb_act);

      if (ev == E_OK) {
        (void)osEE_scheduler_task_activated(p_kdb, p_tdb_act);
      }

      osEE_end_primitive(flags);
    } else {
      ev = E_OS_ID;
    }
  }

#if (defined(OSEE_HAS_ERRORHOOK))
  if (ev != E_OK) {
    VAR(OsEE_api_param, AUTOMATIC)
      param;
#if (!defined(OSEE_HAS_ORTI))
    CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA)
      p_ccb = osEE_get_curr_core()->p_ccb;
#endif /* !OSEE_HAS_ORTI */
    CONST(OsEE_reg, AUTOMATIC)
      flags = osEE_begin_primitive();
    osEE_set_service_id(p_ccb, OSServiceId_ActivateTask);
    param.num_param = TaskID;
    osEE_set_api_param1(p_ccb, param);
    osEE_call_error_hook(p_ccb, ev);
    osEE_end_primitive(flags);
  }
#endif /* OSEE_HAS_ERRORHOOK */
#if (defined(OSEE_HAS_ORTI))
  osEE_orti_trace_service_exit(p_ccb, OSServiceId_ActivateTask);
#endif /* OSEE_HAS_ORTI */

  return ev;
}

FUNC(StatusType, OS_CODE)
  ChainTask
(
  VAR(TaskType, AUTOMATIC) TaskID
)
{
  VAR(StatusType, AUTOMATIC)                    ev;
  CONSTP2VAR(OsEE_KDB, AUTOMATIC, OS_APPL_DATA) p_kdb = osEE_get_kernel();
#if (defined(OSEE_HAS_ORTI))
  CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA)
    p_ccb = osEE_get_curr_core()->p_ccb;
  osEE_orti_trace_service_entry(p_ccb, OSServiceId_ChainTask);
#endif /* OSEE_HAS_ORTI */

  if (!osEE_is_valid_tid(p_kdb, TaskID)) {
    ev = E_OS_ID;
  } else {
    CONSTP2VAR(OsEE_TDB, AUTOMATIC, OS_APPL_DATA)
      p_tdb_act = (*p_kdb->p_tdb_ptr_array)[TaskID];
    CONSTP2VAR(OsEE_TDB, AUTOMATIC, OS_APPL_DATA)
      p_curr = osEE_get_curr_task();
#if (defined(OSEE_HAS_CHECKS)) && (defined(OSEE_HAS_MUTEX))
    CONSTP2VAR(OsEE_TCB, AUTOMATIC, OS_APPL_DATA)
      p_curr_tcb  = p_curr->p_tcb;
#endif /* OSEE_HAS_CHECKS && OSEE_HAS_CHECKS */
#if (defined(OSEE_HAS_CHECKS))
    if (p_curr->task_type >= OSEE_TASK_TYPE_ISR2) {
      ev = E_OS_CALLEVEL;
    } else
#if (defined(OSEE_HAS_MUTEX))
    if (p_curr_tcb->p_first_mtx != NULL) {
#if (!defined(OSEE_SINGLECORE))
    if (p_curr_tcb->p_first_mtx->mtx_type == OSEE_MUTEX_SPINLOCK) {
      ev = E_OS_SPINLOCK;
    } else
#endif /* !OSEE_SINGLECORE */
    {
      ev = E_OS_RESOURCE;
    }
  } else
#endif /* OSEE_HAS_MUTEX */
#endif /* OSEE_HAS_CHECKS */
    if (p_tdb_act->task_type <= OSEE_TASK_TYPE_EXTENDED) {
      CONST(OsEE_reg, AUTOMATIC)  flags = osEE_begin_primitive();

      if (p_tdb_act == p_curr) {
        /* If the Task chain on it self, flag it. */
        p_tdb_act->p_tcb->status = OSEE_TASK_CHAINED;
        ev = E_OK;
      } else {
        ev = osEE_task_activated(p_tdb_act);
        if (ev == E_OK) {
          (void)osEE_scheduler_task_insert(p_kdb, p_tdb_act);
        }
      }
      if (ev == E_OK) {
        /* The following do not return! */
        osEE_hal_terminate_activation(&osEE_get_curr_task()->hdb,
          OSEE_KERNEL_TERMINATE_ACTIVATION_CB);
      }
      osEE_end_primitive(flags);
    } else {
      ev = E_OS_ID;
    }
  }

#if (defined(OSEE_HAS_ERRORHOOK))
  if (ev != E_OK) {
    VAR(OsEE_api_param, AUTOMATIC)
      param;
#if (!defined(OSEE_HAS_ORTI))
    CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA)
      p_ccb = osEE_get_curr_core()->p_ccb;
#endif /* !OSEE_HAS_ORTI */
    CONST(OsEE_reg, AUTOMATIC)
      flags = osEE_begin_primitive();
    osEE_set_service_id(p_ccb, OSServiceId_ChainTask);
    param.num_param = TaskID;
    osEE_set_api_param1(p_ccb, param);
    osEE_call_error_hook(p_ccb, ev);
    osEE_end_primitive(flags);
  }
#endif /* OSEE_HAS_ERRORHOOK */
#if (defined(OSEE_HAS_ORTI))
  osEE_orti_trace_service_exit(p_ccb, OSServiceId_ChainTask);
#endif /* OSEE_HAS_ORTI */

  return ev;
}

FUNC(StatusType, OS_CODE)
  TerminateTask
(
  void
)
{
  VAR(StatusType, AUTOMATIC)  ev;
  CONSTP2VAR(OsEE_CDB, AUTOMATIC, OS_APPL_DATA)
    p_cdb       = osEE_get_curr_core();
  CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA)
    p_ccb       = p_cdb->p_ccb;
  CONSTP2VAR(OsEE_TDB, AUTOMATIC, OS_APPL_DATA)
    p_curr      = p_ccb->p_curr;
#if (defined(OSEE_HAS_CHECKS))
#if (defined(OSEE_HAS_MUTEX))
  CONSTP2VAR(OsEE_TCB, AUTOMATIC, OS_APPL_DATA)
    p_curr_tcb  = p_curr->p_tcb;
#endif /* OSEE_HAS_MUTEX */

  osEE_orti_trace_service_entry(p_ccb, OSServiceId_TerminateTask);

  /*  [OS_SWS_088]: If an OS-Application makes a service call from the wrong
   *  context AND is currently not inside a Category 1 ISR the Operating
   *  System module shall not perform the requested action
   *  (the service call shall have no effect), and return E_OS_CALLEVEL
   *  (see [12], section 13.1) or the "invalid value" of  the service.
   *  (BSW11009, BSW11013) */
  /* check for a call at interrupt level
   * This must be the FIRST Check!!!
   */
  if ((p_curr->task_type >= OSEE_TASK_TYPE_ISR2)
#if (!defined(OSEE_SERVICE_PROTECTION))
  )
#else
    || (p_ccb->os_context > OSEE_TASK_CTX)
  )
#endif /* OSEE_SERVICE_PROTECTION */
  {
    ev = E_OS_CALLEVEL;
  } else
#if (defined(OSEE_HAS_MUTEX))
  if (p_curr_tcb->p_first_mtx != NULL) {
#if (!defined(OSEE_SINGLECORE))
    if (p_curr_tcb->p_first_mtx->mtx_type == OSEE_MUTEX_SPINLOCK) {
      ev = E_OS_SPINLOCK;
    } else
#endif /* !OSEE_SINGLECORE */
    {
      ev = E_OS_RESOURCE;
    }
  } else
#endif /* OSEE_HAS_MUTEX */
#endif /* OSEE_HAS_CHECKS */
  {
    CONST(OsEE_reg, AUTOMATIC)
      flags = osEE_begin_primitive();

    /* The following does not return! */
    osEE_hal_terminate_activation(
      &p_curr->hdb, OSEE_KERNEL_TERMINATE_ACTIVATION_CB
    );

    osEE_end_primitive(flags);

    ev = E_OK;
  }

#if (defined(OSEE_HAS_ERRORHOOK))
  if (ev != E_OK) {
    CONST(OsEE_reg, AUTOMATIC)
      flags = osEE_begin_primitive();
    osEE_set_service_id(p_ccb, OSServiceId_TerminateTask);
    osEE_call_error_hook(p_ccb, ev);
    osEE_end_primitive(flags);
  }
#endif /* OSEE_HAS_ERRORHOOK */

  osEE_orti_trace_service_exit(p_ccb, OSServiceId_TerminateTask);

  return ev;
}

FUNC(StatusType, OS_CODE)
  Schedule
(
  void
)
{
  VAR(StatusType, AUTOMATIC)                    ev;
  CONSTP2VAR(OsEE_CDB, AUTOMATIC, OS_APPL_DATA) p_cdb   = osEE_get_curr_core();
  CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA) p_ccb   = p_cdb->p_ccb;
  CONSTP2VAR(OsEE_TDB, AUTOMATIC, OS_APPL_DATA) p_curr  = p_ccb->p_curr;
  CONSTP2VAR(OsEE_TCB, AUTOMATIC, OS_APPL_DATA) p_tcb   = p_curr->p_tcb;

  osEE_orti_trace_service_entry(p_ccb, OSServiceId_Schedule);

#if (defined(OSEE_HAS_CHECKS))
  if (p_curr->task_type > OSEE_TASK_TYPE_EXTENDED) {
    ev = E_OS_CALLEVEL;
  } else
#if (defined(OSEE_HAS_MUTEX))
  if (p_tcb->p_first_mtx != NULL) {
    ev = E_OS_RESOURCE;
  } else
#endif /* OSEE_HAS_MUTEX */
#endif /* OSEE_HAS_CHECKS */
  if (p_tcb->current_prio == p_curr->dispatch_prio)
  {
    /* Begin primitive */
    CONST(OsEE_reg, AUTOMATIC)  flags = osEE_begin_primitive();

    /* Release internal resources */
    p_tcb->current_prio = p_curr->ready_prio;
    /* Try preemption */
    osEE_scheduler_task_preemption_point(osEE_get_kernel());
    /* Restore internal resources */
    p_tcb->current_prio = p_curr->dispatch_prio;

    /* End primitives */
    osEE_end_primitive(flags);

    ev = E_OK;
  } else {
    /* Ignore Schedule if the TASK got an explicit Resource */
    ev = E_OK;
  }

#if (defined(OSEE_HAS_ERRORHOOK))
  if (ev != E_OK) {
    CONST(OsEE_reg, AUTOMATIC)
      flags = osEE_begin_primitive();
    osEE_set_service_id(p_ccb, OSServiceId_Schedule);
    osEE_call_error_hook(p_ccb, ev);
    osEE_end_primitive(flags);
  }
#endif /* OSEE_HAS_ERRORHOOK */

  osEE_orti_trace_service_exit(p_ccb, OSServiceId_Schedule);

  return ev;
}

#if (defined(OSEE_HAS_RESOURCES))
FUNC(StatusType, OS_CODE)
  GetResource
(
  VAR(ResourceType, AUTOMATIC) ResID
)
{
  VAR(StatusType, AUTOMATIC)                    ev;
  CONSTP2VAR(OsEE_KDB, AUTOMATIC, OS_APPL_DATA) p_kdb = osEE_get_kernel();
#if (defined(OSEE_HAS_ORTI))
  CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA)
    p_ccb = osEE_get_curr_core()->p_ccb;
  osEE_orti_trace_service_entry(p_ccb, OSServiceId_GetResource);
#endif /* OSEE_HAS_ORTI */

  if (!osEE_is_valid_res_id(p_kdb, ResID)) {
    ev = E_OS_ID;
  } else
  {
    CONSTP2VAR(OsEE_MDB, AUTOMATIC, TYPEDEF)
      p_mtx     = (*p_kdb->p_res_ptr_array)[ResID];
    CONSTP2VAR(OsEE_MCB, AUTOMATIC, TYPEDEF)
      p_mtx_mcb = p_mtx->p_mcb;
    CONSTP2VAR(OsEE_TDB, AUTOMATIC, TYPEDEF)
      p_tdb     = osEE_get_curr_task();
    CONSTP2VAR(OsEE_TCB, AUTOMATIC, TYPEDEF)
      p_tcb     = p_tdb->p_tcb;
    CONST(TaskPrio, AUTOMATIC)
      mtx_prio  = p_mtx->mtx_prio;
    VAR(OsEE_reg, AUTOMATIC)
      flags = osEE_begin_primitive();

#if (defined(OSEE_HAS_CHECKS))
    if ((p_mtx_mcb->locked) || (p_tdb->ready_prio > mtx_prio)) {
      osEE_end_primitive(flags);

      ev = E_OS_ACCESS;
    } else
#endif /* OSEE_HAS_CHECKS */
    {
      CONST(TaskPrio, AUTOMATIC)
        current_prio = p_tcb->current_prio;

      if (current_prio < mtx_prio) {
        p_tcb->current_prio = mtx_prio;
        flags = osEE_hal_prepare_ipl(flags, mtx_prio);
      }

      p_mtx_mcb->p_next       = p_tcb->p_first_mtx;
      p_mtx_mcb->prev_prio    = current_prio;
#if (defined(OSEE_HAS_CHECKS)) || (defined(OSEE_HAS_ORTI))
      p_mtx_mcb->locked       = OSEE_TRUE;
#endif /* OSEE_HAS_CHECKS || OSEE_HAS_ORTI */
#if (!defined(OSEE_SINGLECORE)) || (defined(OSEE_HAS_ORTI))
      p_mtx_mcb->p_mtx_owner  = p_tdb;
#endif /* !OSEE_SINGLECORE || OSEE_HAS_ORTI */
      p_tcb->p_first_mtx      = p_mtx;

      osEE_end_primitive(flags);

      ev = E_OK;
    }
  }

#if (defined(OSEE_HAS_ERRORHOOK))
  if (ev != E_OK) {
    VAR(OsEE_api_param, AUTOMATIC)
      param;
#if (!defined(OSEE_HAS_ORTI))
    CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA)
      p_ccb = osEE_get_curr_core()->p_ccb;
#endif /* !OSEE_HAS_ORTI */
    CONST(OsEE_reg, AUTOMATIC)
      flags = osEE_begin_primitive();
    osEE_set_service_id(p_ccb, OSServiceId_GetResource);
    param.num_param = ResID;
    osEE_set_api_param1(p_ccb, param);
    osEE_call_error_hook(p_ccb, ev);
    osEE_end_primitive(flags);
  }
#endif /* OSEE_HAS_ERRORHOOK */
#if (defined(OSEE_HAS_ORTI))
  osEE_orti_trace_service_exit(p_ccb, OSServiceId_GetResource);
#endif /* OSEE_HAS_ORTI */

  return ev;
}

FUNC(StatusType, OS_CODE)
  ReleaseResource
(
  VAR(ResourceType, AUTOMATIC) ResID
)
{
  VAR(StatusType, AUTOMATIC)                    ev;
  CONSTP2VAR(OsEE_KDB, AUTOMATIC, OS_APPL_DATA) p_kdb = osEE_get_kernel();
  CONSTP2VAR(OsEE_CDB, AUTOMATIC, OS_APPL_DATA) p_cdb = osEE_get_curr_core();
#if (defined(OSEE_HAS_ORTI))
  CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA) p_ccb = p_cdb->p_ccb;
  osEE_orti_trace_service_entry(p_ccb, OSServiceId_ReleaseResource);
#endif /* OSEE_HAS_ORTI */

  if (!osEE_is_valid_res_id(p_kdb, ResID)) {
    ev = E_OS_ID;
  } else
  {
    CONSTP2VAR(OsEE_TDB, AUTOMATIC, TYPEDEF)
      p_tdb     = osEE_get_curr_task();
    CONSTP2VAR(OsEE_TCB, AUTOMATIC, TYPEDEF)
      p_tcb     = p_tdb->p_tcb;
    CONSTP2VAR(OsEE_MDB, AUTOMATIC, TYPEDEF)
      p_mtx     = (*p_kdb->p_res_ptr_array)[ResID];
    CONSTP2VAR(OsEE_MCB, AUTOMATIC, TYPEDEF)
      p_mtx_mcb = p_mtx->p_mcb;

#if (defined(OSEE_HAS_CHECKS))
    if ((p_mtx_mcb->locked == OSEE_FALSE) || (p_tcb->p_first_mtx != p_mtx)) {
      ev = E_OS_NOFUNC;
    } else
#endif /* OSEE_HAS_CHECKS */
    {
      VAR(OsEE_reg, AUTOMATIC)
        flags = osEE_begin_primitive();
      /* Pop the MTX head */
      p_tcb->p_first_mtx = p_tcb->p_first_mtx->p_mcb->p_next;

      if (p_tcb->p_first_mtx != NULL) {
        CONST(TaskPrio, AUTOMATIC)
          prev_prio = p_mtx_mcb->prev_prio;

        p_tcb->current_prio = prev_prio;
        flags = osEE_hal_prepare_ipl(flags, prev_prio);
      } else {
        CONST(TaskPrio, AUTOMATIC)
          dispatch_prio = p_tdb->dispatch_prio;

        p_tcb->current_prio = dispatch_prio;
        flags = osEE_hal_prepare_ipl(flags, dispatch_prio);
      }
#if (defined(OSEE_HAS_CHECKS)) || (defined(OSEE_HAS_ORTI))
      p_mtx_mcb->locked       = OSEE_FALSE;
#endif /* OSEE_HAS_CHECKS || OSEE_HAS_ORTI */
#if (!defined(OSEE_SINGLECORE)) || (defined(OSEE_HAS_ORTI))
      p_mtx_mcb->p_mtx_owner  = NULL;
#endif /* !OSEE_SINGLECORE || OSEE_HAS_ORTI */

      /* Preemption point */
      (void)osEE_scheduler_task_preemption_point(p_kdb);

      osEE_end_primitive(flags);

      ev = E_OK;
    }
  }

#if (defined(OSEE_HAS_ERRORHOOK))
  if (ev != E_OK) {
    VAR(OsEE_api_param, AUTOMATIC)
      param;
#if (!defined(OSEE_HAS_ORTI))
    CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA)
      p_ccb = p_cdb->p_ccb;
#endif /* !OSEE_HAS_ORTI */
    CONST(OsEE_reg, AUTOMATIC)
      flags = osEE_begin_primitive();
    osEE_set_service_id(p_ccb, OSServiceId_ReleaseResource);
    param.num_param = ResID;
    osEE_set_api_param1(p_ccb, param);
    osEE_call_error_hook(p_ccb, ev);
    osEE_end_primitive(flags);
  }
#endif /* OSEE_HAS_ERRORHOOK */
#if (defined(OSEE_HAS_ORTI))
  osEE_orti_trace_service_exit(p_ccb, OSServiceId_ReleaseResource);
#endif /* OSEE_HAS_ORTI */
  return ev;
}
#endif /* OSEE_HAS_RESOURCES */

FUNC(StatusType, OS_CODE)
  ShutdownOS
(
  VAR(StatusType, AUTOMATIC) Error
)
{
  VAR(StatusType, AUTOMATIC)                    ev;
  CONSTP2VAR(OsEE_CDB, AUTOMATIC, OS_APPL_DATA) p_cdb = osEE_get_curr_core();
  CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA) p_ccb = p_cdb->p_ccb;
  CONST(OsEE_reg, AUTOMATIC)  flags = osEE_begin_primitive();
  CONST(OsEE_kernel_status, AUTOMATIC) os_status = p_ccb->os_status;

  osEE_orti_trace_service_entry(p_ccb, OSServiceId_ShutdownOS);

  if ((os_status == OSEE_KERNEL_STARTED) || (os_status == OSEE_KERNEL_STARTING))
  {
    p_ccb->os_status = OSEE_KERNEL_SHUTDOWN;
    /* Used to propagate the error to the ShutdownHook */
    p_ccb->last_error = Error;

#if (defined(OSEE_SHUTDOWN_DO_NOT_RETURN_ON_MAIN))
    osEE_hal_disableIRQ();
    osEE_call_shutdown_hook(p_ccb, Error);
    while (1) {
      ; /* Endless Loop */
    }
#else
    if (os_status == OSEE_KERNEL_STARTED) {
      osEE_idle_task_terminate(p_cdb->p_idle_task);
    }
    ev = E_OK;
#endif /* OSEE_SHUTDOWN_DO_NOT_RETURN_ON_MAIN */
  } else {
    ev = E_OS_STATE;
  }

  if (ev != E_OK) {
    osEE_call_error_hook(p_ccb, ev);
  }

  osEE_orti_trace_service_exit(p_ccb, OSServiceId_ShutdownOS);
  osEE_end_primitive(flags);

  return ev;
}

FUNC(StatusType, OS_CODE)
  GetTaskID
(
  VAR(TaskRefType, AUTOMATIC) TaskID
)
{
  VAR(StatusType, AUTOMATIC) ev;
#if (defined(OSEE_HAS_ORTI))
  CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA)
    p_ccb = osEE_get_curr_core()->p_ccb;
  osEE_orti_trace_service_entry(p_ccb, OSServiceId_GetTaskID);
#endif /* OSEE_HAS_ORTI */
  /* [OS566]: The Operating System API shall check in extended mode all pointer
      argument for NULL pointer and return OS_E_PARAMETER_POINTER
      if such argument is NULL.
      +
      MISRA dictate NULL check for pointers always. */
  if (TaskID == NULL) {
    ev = E_OS_PARAM_POINTER;
  } else {
    VAR(TaskType, AUTOMATIC)
      tid = INVALID_TASK;
    CONSTP2CONST(OsEE_TDB, AUTOMATIC, OS_APPL_DATA)
      p_tdb = osEE_get_curr_task();

    if (p_tdb->task_type <= OSEE_TASK_TYPE_EXTENDED) {
      tid = p_tdb->tid;
    } else if (p_tdb->task_type == OSEE_TASK_TYPE_ISR2) {
      CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA)
        p_ccb = osEE_get_curr_core()->p_ccb;
      /* In case of ISR2 search the first stacked that is not an ISR2 */
      P2VAR(OsEE_SN, AUTOMATIC, OS_APPL_DATA)
        p_sn = p_ccb->p_stk_sn->p_next;

      while (p_sn != NULL) {
        CONSTP2CONST(OsEE_TDB, AUTOMATIC, OS_APPL_DATA)
          p_searched_tdb = p_sn->p_tdb;
        if (p_searched_tdb->task_type <= OSEE_TASK_TYPE_EXTENDED) {
          tid = p_searched_tdb->tid;
          break;
        } else {
          p_sn = p_sn->p_next;
        }
      }
    }
    /* XXX: This SHALL be atomic. */
    (*TaskID) = tid;
    ev = E_OK;
  }

#if (defined(OSEE_HAS_ERRORHOOK))
  if (ev != E_OK) {
    VAR(OsEE_api_param, AUTOMATIC)
      param;
#if (!defined(OSEE_HAS_ORTI))
    CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA)
      p_ccb = osEE_get_curr_core()->p_ccb;
#endif /* !OSEE_HAS_ORTI */
    CONST(OsEE_reg, AUTOMATIC)
      flags = osEE_begin_primitive();
    osEE_set_service_id(p_ccb, OSServiceId_GetTaskID);
    param.p_param = TaskID;
    osEE_set_api_param1(p_ccb, param);
    osEE_call_error_hook(p_ccb, ev);
    osEE_end_primitive(flags);
  }
#endif /* OSEE_HAS_ERRORHOOK */
#if (defined(OSEE_HAS_ORTI))
  osEE_orti_trace_service_exit(p_ccb, OSServiceId_GetTaskID);
#endif /* OSEE_HAS_ORTI */

  return ev;
}

FUNC(StatusType, OS_CODE)
  GetTaskState
(
  VAR(TaskType, AUTOMATIC)          TaskID,
  VAR(TaskStateRefType, AUTOMATIC)  State
)
{
  VAR(StatusType, AUTOMATIC)                    ev;
  CONSTP2VAR(OsEE_KDB, AUTOMATIC, OS_APPL_DATA) p_kdb = osEE_get_kernel();
#if (defined(OSEE_HAS_ORTI))
  CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA)
    p_ccb = osEE_get_curr_core()->p_ccb;
  osEE_orti_trace_service_entry(p_ccb, OSServiceId_GetTaskState);
#endif /* OSEE_HAS_ORTI */

  /* [OS566]: The Operating System API shall check in extended mode all pointer
     argument for NULL pointer and return OS_E_PARAMETER_POINTER
     if such argument is NULL.
     +
     MISRA dictate NULL check for pointers always. */
  if (State == NULL) {
    ev = E_OS_PARAM_POINTER;
  } else
  if (!osEE_is_valid_tid(p_kdb, TaskID)) {
    ev = E_OS_ID;
  } else
  {
    CONSTP2VAR(OsEE_TDB, AUTOMATIC, OS_APPL_DATA)
      p_tdb = (*p_kdb->p_tdb_ptr_array)[TaskID];
    /* XXX: This SHALL be atomic. Sure for TriCore,
            visually check generate asm for each architecture */
    CONST(TaskType, AUTOMATIC) local_state = p_tdb->p_tcb->status;
    switch (local_state) {
      case OSEE_TASK_SUSPENDED:
        (*State) = SUSPENDED;
        break;
      case OSEE_TASK_READY:
      case OSEE_TASK_READY_STACKED:
        (*State) = READY;
        break;
      case OSEE_TASK_WAITING:
        (*State) = WAITING;
        break;
      case OSEE_TASK_RUNNING:
      case OSEE_TASK_CHAINED:
        (*State) = RUNNING;
        break;
      default:
        OSEE_RUN_ASSERT(OSEE_FALSE,"Invalid Task State");
        break;
    }
    ev = E_OK;
  }

#if (defined(OSEE_HAS_ERRORHOOK))
  if (ev != E_OK) {
    VAR(OsEE_api_param, AUTOMATIC)
      param;
#if (!defined(OSEE_HAS_ORTI))
    CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA)
      p_ccb = osEE_get_curr_core()->p_ccb;
#endif /* !OSEE_HAS_ORTI */
    CONST(OsEE_reg, AUTOMATIC)
      flags = osEE_begin_primitive();
    osEE_set_service_id(p_ccb, OSServiceId_GetTaskState);
    param.num_param = TaskID;
    osEE_set_api_param1(p_ccb, param);
    param.p_param = State;
    osEE_set_api_param2(p_ccb, param);
    osEE_call_error_hook(p_ccb, ev);
    osEE_end_primitive(flags);
  }
#endif /* OSEE_HAS_ERRORHOOK */
#if (defined(OSEE_HAS_ORTI))
  osEE_orti_trace_service_exit(p_ccb, OSServiceId_GetTaskState);
#endif /* OSEE_HAS_ORTI */

  return ev;
}

#if (defined(OSEE_HAS_ALARMS))
FUNC(StatusType, OS_CODE)
  SetRelAlarm
(
  VAR(AlarmType,  AUTOMATIC)  AlarmID,
  VAR(TickType,   AUTOMATIC)  increment,
  VAR(TickType,   AUTOMATIC)  cycle
)
{
  VAR(StatusType, AUTOMATIC)  ev;
  CONSTP2VAR(OsEE_KDB, AUTOMATIC, OS_APPL_DATA)
    p_kdb = osEE_get_kernel();
#if (defined(OSEE_HAS_ORTI))
  CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA)
    p_ccb = osEE_get_curr_core()->p_ccb;
  osEE_orti_trace_service_entry(p_ccb, OSServiceId_SetRelAlarm);
#endif /* OSEE_HAS_ORTI */

  if (!osEE_is_valid_alarm_id(p_kdb, AlarmID)) {
    ev = E_OS_ID;
  } else
  {
    CONSTP2VAR(OsEE_AlarmDB, AUTOMATIC, OS_APPL_DATA)
      p_alarm_db = (*p_kdb->p_alarm_ptr_array)[AlarmID];
    CONSTP2VAR(OsEE_CounterDB, AUTOMATIC, OS_APPL_DATA)
      p_counter_db = osEE_alarm_get_trigger_db(p_alarm_db)->p_counter_db;

#if (defined(OSEE_HAS_CHECKS))
    /* SWS_Os_00304 */
    if ((increment == 0U) ||
        (increment > p_counter_db->info.maxallowedvalue) ||
        ((cycle != 0U) && ((cycle < p_counter_db->info.mincycle) ||
          (cycle > p_counter_db->info.maxallowedvalue)))
      )
    {
      ev = E_OS_VALUE;
    } else
#endif /* OSEE_HAS_CHECKS */
    {
      CONST(OsEE_reg, AUTOMATIC)
        flags = osEE_begin_primitive();

      ev = osEE_alarm_set_rel(p_counter_db, p_alarm_db, increment, cycle);

      osEE_end_primitive(flags);
    }
  }

#if (defined(OSEE_HAS_ERRORHOOK))
  if (ev != E_OK) {
    VAR(OsEE_api_param, AUTOMATIC)
      param;
#if (!defined(OSEE_HAS_ORTI))
    CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA)
      p_ccb = osEE_get_curr_core()->p_ccb;
#endif /* !OSEE_HAS_ORTI */
    CONST(OsEE_reg, AUTOMATIC)
      flags = osEE_begin_primitive();
    osEE_set_service_id(p_ccb, OSServiceId_SetRelAlarm);
    param.num_param = AlarmID;
    osEE_set_api_param1(p_ccb, param);
    param.num_param = increment;
    osEE_set_api_param2(p_ccb, param);
    param.num_param = cycle;
    osEE_set_api_param3(p_ccb, param);
    osEE_call_error_hook(p_ccb, ev);
    osEE_end_primitive(flags);
  }
#endif /* OSEE_HAS_ERRORHOOK */
#if (defined(OSEE_HAS_ORTI))
  osEE_orti_trace_service_exit(p_ccb, OSServiceId_SetRelAlarm);
#endif /* OSEE_HAS_ORTI */

  return ev;
}

FUNC(StatusType, OS_CODE)
  SetAbsAlarm
(
  VAR(AlarmType,  AUTOMATIC)  AlarmID,
  VAR(TickType,   AUTOMATIC)  start,
  VAR(TickType,   AUTOMATIC)  cycle
)
{
  VAR(StatusType, AUTOMATIC)  ev;
  CONSTP2VAR(OsEE_KDB, AUTOMATIC, OS_APPL_DATA)
    p_kdb = osEE_get_kernel();
#if (defined(OSEE_HAS_ORTI))
  CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA)
    p_ccb = osEE_get_curr_core()->p_ccb;
  osEE_orti_trace_service_entry(p_ccb, OSServiceId_SetAbsAlarm);
#endif /* OSEE_HAS_ORTI */

  if (!osEE_is_valid_alarm_id(p_kdb, AlarmID)) {
    ev = E_OS_ID;
  } else
  {
    CONSTP2VAR(OsEE_AlarmDB, AUTOMATIC, OS_APPL_DATA)
      p_alarm_db = (*p_kdb->p_alarm_ptr_array)[AlarmID];
    CONSTP2VAR(OsEE_CounterDB, AUTOMATIC, OS_APPL_DATA)
      p_counter_db = osEE_alarm_get_trigger_db(p_alarm_db)->p_counter_db;

#if (defined(OSEE_HAS_CHECKS))
    /* SWS_Os_00304 */
    if ((start > p_counter_db->info.maxallowedvalue) ||
        ((cycle != 0U) && ((cycle < p_counter_db->info.mincycle) ||
          (cycle > p_counter_db->info.maxallowedvalue)))
      )
    {
      ev = E_OS_VALUE;
    } else
#endif /* OSEE_HAS_CHECKS */
    {
      CONST(OsEE_reg, AUTOMATIC)
        flags = osEE_begin_primitive();

      ev = osEE_alarm_set_abs(p_counter_db, p_alarm_db, start, cycle);

      osEE_end_primitive(flags);
    }
  }

#if (defined(OSEE_HAS_ERRORHOOK))
  if (ev != E_OK) {
    VAR(OsEE_api_param, AUTOMATIC)
      param;
#if (!defined(OSEE_HAS_ORTI))
    CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA)
      p_ccb = osEE_get_curr_core()->p_ccb;
#endif /* !OSEE_HAS_ORTI */
    CONST(OsEE_reg, AUTOMATIC)
      flags = osEE_begin_primitive();
    osEE_set_service_id(p_ccb, OSServiceId_SetAbsAlarm);
    param.num_param = AlarmID;
    osEE_set_api_param1(p_ccb, param);
    param.num_param = start;
    osEE_set_api_param2(p_ccb, param);
    param.num_param = cycle;
    osEE_set_api_param3(p_ccb, param);
    osEE_call_error_hook(p_ccb, ev);
    osEE_end_primitive(flags);
  }
#endif /* OSEE_HAS_ERRORHOOK */
#if (defined(OSEE_HAS_ORTI))
  osEE_orti_trace_service_exit(p_ccb, OSServiceId_SetAbsAlarm);
#endif /* OSEE_HAS_ORTI */

  return ev;
}

FUNC(StatusType, OS_CODE)
  CancelAlarm
(
  VAR(AlarmType, AUTOMATIC) AlarmID
)
{
  VAR(StatusType, AUTOMATIC)  ev;
  CONSTP2VAR(OsEE_KDB, AUTOMATIC, OS_APPL_DATA)
    p_kdb = osEE_get_kernel();
#if (defined(OSEE_HAS_ORTI))
  CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA)
    p_ccb = osEE_get_curr_core()->p_ccb;
  osEE_orti_trace_service_entry(p_ccb, OSServiceId_CancelAlarm);
#endif /* OSEE_HAS_ORTI */

  if (!osEE_is_valid_alarm_id(p_kdb, AlarmID)) {
    ev = E_OS_ID;
  } else {
    CONSTP2VAR(OsEE_AlarmDB, AUTOMATIC, OS_APPL_DATA)
      p_alarm_db  = (*p_kdb->p_alarm_ptr_array)[AlarmID];
    CONST(OsEE_reg, AUTOMATIC)
      flags = osEE_begin_primitive();

    ev = osEE_alarm_cancel(p_alarm_db);

    osEE_end_primitive(flags);
  }

#if (defined(OSEE_HAS_ERRORHOOK))
  if (ev != E_OK) {
    VAR(OsEE_api_param, AUTOMATIC)
      param;
#if (!defined(OSEE_HAS_ORTI))
    CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA)
      p_ccb = osEE_get_curr_core()->p_ccb;
#endif /* !OSEE_HAS_ORTI */
    CONST(OsEE_reg, AUTOMATIC)
      flags = osEE_begin_primitive();
    osEE_set_service_id(p_ccb, OSServiceId_CancelAlarm);
    param.num_param = AlarmID;
    osEE_set_api_param1(p_ccb, param);
    osEE_call_error_hook(p_ccb, ev);
    osEE_end_primitive(flags);
  }
#endif /* OSEE_HAS_ERRORHOOK */
#if (defined(OSEE_HAS_ORTI))
  osEE_orti_trace_service_exit(p_ccb, OSServiceId_CancelAlarm);
#endif /* OSEE_HAS_ORTI */

  return ev;
}

FUNC(StatusType, OS_CODE)
  GetAlarm
(
  VAR(AlarmType, AUTOMATIC)   AlarmID,
  VAR(TickRefType, AUTOMATIC) Tick
)
{
  VAR(StatusType, AUTOMATIC)  ev;
  CONSTP2VAR(OsEE_KDB, AUTOMATIC, OS_APPL_DATA)
    p_kdb = osEE_get_kernel();
#if (defined(OSEE_HAS_ORTI))
  CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA)
    p_ccb = osEE_get_curr_core()->p_ccb;
  osEE_orti_trace_service_entry(p_ccb, OSServiceId_GetAlarm);
#endif /* OSEE_HAS_ORTI */

  if (!osEE_is_valid_alarm_id(p_kdb, AlarmID)) {
    ev = E_OS_ID;
  } else
  if (Tick == NULL) {
    ev = E_OS_PARAM_POINTER;
  } else
  {
    CONSTP2VAR(OsEE_AlarmDB, AUTOMATIC, OS_APPL_DATA)
      p_alarm_db = (*p_kdb->p_alarm_ptr_array)[AlarmID];
    CONST(OsEE_reg, AUTOMATIC)
      flags = osEE_begin_primitive();

    ev = osEE_alarm_get(p_alarm_db, Tick);

    osEE_end_primitive(flags);
  }

#if (defined(OSEE_HAS_ERRORHOOK))
  if (ev != E_OK) {
    VAR(OsEE_api_param, AUTOMATIC)
      param;
#if (!defined(OSEE_HAS_ORTI))
    CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA)
      p_ccb = osEE_get_curr_core()->p_ccb;
#endif /* !OSEE_HAS_ORTI */
    CONST(OsEE_reg, AUTOMATIC)
      flags = osEE_begin_primitive();
    osEE_set_service_id(p_ccb, OSServiceId_GetAlarm);
    param.num_param = AlarmID;
    osEE_set_api_param1(p_ccb, param);
    param.p_param = Tick;
    osEE_set_api_param2(p_ccb, param);
    osEE_call_error_hook(p_ccb, ev);
    osEE_end_primitive(flags);
  }
#endif /* OSEE_HAS_ERRORHOOK */
#if (defined(OSEE_HAS_ORTI))
  osEE_orti_trace_service_exit(p_ccb, OSServiceId_GetAlarm);
#endif /* OSEE_HAS_ORTI */

  return ev;
}

FUNC(StatusType, OS_CODE)
  GetAlarmBase
(
  VAR(AlarmType, AUTOMATIC)         AlarmID,
  VAR(AlarmBaseRefType, AUTOMATIC)  Info
)
{
  VAR(StatusType, AUTOMATIC)  ev;
  CONSTP2VAR(OsEE_KDB, AUTOMATIC, OS_APPL_DATA)
    p_kdb = osEE_get_kernel();
#if (defined(OSEE_HAS_ORTI))
  CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA)
    p_ccb = osEE_get_curr_core()->p_ccb;
  osEE_orti_trace_service_entry(p_ccb, OSServiceId_GetAlarmBase);
#endif /* OSEE_HAS_ORTI */

  if (!osEE_is_valid_alarm_id(p_kdb, AlarmID)) {
    ev = E_OS_ID;
  } else
  if (Info == NULL) {
    ev = E_OS_PARAM_POINTER;
  } else
  {
    CONSTP2VAR(OsEE_AlarmDB, AUTOMATIC, OS_APPL_DATA)
      p_alarm_db = (*p_kdb->p_alarm_ptr_array)[AlarmID];
    CONSTP2VAR(OsEE_TriggerDB, AUTOMATIC, OS_APPL_DATA)
      p_trigger_db = osEE_alarm_get_trigger_db(p_alarm_db);
    CONSTP2VAR(OsEE_CounterDB, AUTOMATIC, OS_APPL_DATA)
      p_counter_db = p_trigger_db->p_counter_db;

    *Info = p_counter_db->info;

    ev = E_OK;
  }

#if (defined(OSEE_HAS_ERRORHOOK))
  if (ev != E_OK) {
    VAR(OsEE_api_param, AUTOMATIC)
      param;
#if (!defined(OSEE_HAS_ORTI))
    CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA)
      p_ccb = osEE_get_curr_core()->p_ccb;
#endif /* !OSEE_HAS_ORTI */
    CONST(OsEE_reg, AUTOMATIC)
      flags = osEE_begin_primitive();
    osEE_set_service_id(p_ccb, OSServiceId_GetAlarmBase);
    param.num_param = AlarmID;
    osEE_set_api_param1(p_ccb, param);
    param.p_param = Info;
    osEE_set_api_param2(p_ccb, param);
    osEE_call_error_hook(osEE_get_curr_core()->p_ccb, ev);
    osEE_end_primitive(flags);
  }
#endif /* OSEE_HAS_ERRORHOOK */
#if (defined(OSEE_HAS_ORTI))
  osEE_orti_trace_service_exit(p_ccb, OSServiceId_GetAlarmBase);
#endif /* OSEE_HAS_ORTI */

  return ev;
}

#endif /* OSEE_HAS_ALARMS */

#if (defined(OSEE_HAS_EVENTS))
FUNC(StatusType, OS_CODE)
  WaitEvent
(
  VAR(EventMaskType, AUTOMATIC) Mask
)
{
  VAR(StatusType, AUTOMATIC)  ev;
  CONSTP2VAR(OsEE_CDB, AUTOMATIC, OS_APPL_DATA)
    p_cdb       = osEE_get_curr_core();
  CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA)
    p_ccb       = p_cdb->p_ccb;
  CONSTP2VAR(OsEE_TDB, AUTOMATIC, OS_APPL_DATA)
    p_curr      = p_ccb->p_curr;
  CONSTP2VAR(OsEE_TCB, AUTOMATIC, OS_APPL_DATA)
    p_curr_tcb  = p_curr->p_tcb;

  osEE_orti_trace_service_entry(p_ccb, OSServiceId_WaitEvent);

#if (defined(OSEE_HAS_CHECKS))
  /*  [OS_SWS_093]: If interrupts are disabled/suspended by a Task/OsIsr and
   *    the Task/OsIsr calls any OS service (excluding the interrupt services)
   *    then the Operating System shall ignore the service AND shall return */
  /*  [OS_SWS_088]: If an OS-Application makes a service call from the wrong
   *    context AND is currently not inside a Category 1 ISR the Operating
   *    System module shall not perform the requested action
   *    (the service call shall have no effect), and return E_OS_CALLEVEL
   *    (see [12], section 13.1) or the "invalid value" of  the service.
   *    (BSW11009, BSW11013) */
  if (osEE_check_disableint(p_ccb)) {
    ev = E_OS_DISABLEDINT;
  } else
  if ((p_curr->task_type >= OSEE_TASK_TYPE_ISR2)
#if (!defined(OSEE_SERVICE_PROTECTION))
  )
#else
    || (p_ccb->os_context > OSEE_TASK_CTX)
  )
#endif /* OSEE_SERVICE_PROTECTION */
  {
    ev = E_OS_CALLEVEL;
  } else
#if (defined(OSEE_HAS_MUTEX))
  if (p_curr_tcb->p_first_mtx != NULL) {
#if (!defined(OSEE_SINGLECORE))
    if (p_curr_tcb->p_first_mtx->mtx_type == OSEE_MUTEX_SPINLOCK) {
      ev = E_OS_SPINLOCK;
    } else
#endif /* !OSEE_SINGLECORE */
    {
      ev = E_OS_RESOURCE;
    }
  } else
  if (p_curr->task_type != OSEE_TASK_TYPE_EXTENDED) {
    ev = E_OS_ACCESS;
  } else
#endif /* OSEE_HAS_MUTEX */
#endif /* OSEE_HAS_CHECKS */
  /* Check if we have to wait */
  if ((p_curr_tcb->event_mask & Mask) == 0U) {
    P2VAR(OsEE_TDB, AUTOMATIC, OS_APPL_DATA)  p_to;
    /* Start Critical Section */
    CONST(OsEE_reg, AUTOMATIC) flags = osEE_begin_primitive();

    /* Set the waiting mask */
    p_curr_tcb->wait_mask = Mask;

    p_to = osEE_scheduler_task_block_current(osEE_get_kernel(),
              &p_curr_tcb->p_own_sn);

    osEE_change_context_from_running(p_curr, p_to);

    /* Reset the waiting mask */
    p_curr_tcb->wait_mask = 0U;

    osEE_end_primitive(flags);

    ev = E_OK;
  } else {
    ev = E_OK;
  }

#if (defined(OSEE_HAS_ERRORHOOK))
  if (ev != E_OK) {
    VAR(OsEE_api_param, AUTOMATIC)
      param;
    CONST(OsEE_reg, AUTOMATIC)
      flags = osEE_begin_primitive();
    osEE_set_service_id(p_ccb, OSServiceId_WaitEvent);
    param.num_param = Mask;
    osEE_set_api_param1(p_ccb, param);
    osEE_call_error_hook(p_ccb, ev);
    osEE_end_primitive(flags);
  }
#endif /* OSEE_HAS_ERRORHOOK */

  osEE_orti_trace_service_exit(p_ccb, OSServiceId_WaitEvent);

  return ev;
}

FUNC(StatusType, OS_CODE)
  SetEvent
(
  VAR(TaskType,      AUTOMATIC) TaskID,
  VAR(EventMaskType, AUTOMATIC) Mask
)
{
  VAR(StatusType, AUTOMATIC)  ev;
  CONSTP2VAR(OsEE_KDB, AUTOMATIC, OS_APPL_DATA)
    p_kdb       = osEE_get_kernel();
#if (defined(OSEE_HAS_CHECKS)) || (defined(OSEE_HAS_ERRORHOOK)) ||\
    (defined(OSEE_HAS_ORTI))
  CONSTP2VAR(OsEE_CDB, AUTOMATIC, OS_APPL_DATA)
    p_curr_cdb  = osEE_get_curr_core();
  CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA)
    p_curr_ccb  = p_curr_cdb->p_ccb;
#if (defined(OSEE_HAS_CHECKS))
  CONSTP2VAR(OsEE_TDB, AUTOMATIC, OS_APPL_DATA)
    p_curr      = p_curr_ccb->p_curr;

  osEE_orti_trace_service_entry(p_curr_ccb, OSServiceId_SetEvent);

  /*  [OS_SWS_093]: If interrupts are disabled/suspended by a Task/OsIsr and
   *    the Task/OsIsr calls any OS service (excluding the interrupt services)
   *    then the Operating System shall ignore the service AND shall return  */
  /*  [OS_SWS_088]: If an OS-Application makes a service call from the wrong
   *    context AND is currently not inside a Category 1 ISR the Operating
   *    System module shall not perform the requested action
   *    (the service call shall have no effect), and return E_OS_CALLEVEL
   *    (see [12], section 13.1) or the "invalid value" of  the service.
   *    (BSW11009, BSW11013) */
/* SetEvent is callable by Task and ISR2 */
  if (osEE_check_disableint(p_curr_ccb)) {
    ev = E_OS_DISABLEDINT;
  } else
  if ((p_curr->task_type > OSEE_TASK_TYPE_ISR2)
#if (!defined(OSEE_SERVICE_PROTECTION))
  )
#else
    || (p_curr_ccb->os_context > OSEE_TASK_ISR2_CTX)
  )
#endif /* OSEE_SERVICE_PROTECTION */
  {
    ev = E_OS_CALLEVEL;
  } else
#elif (defined(OSEE_HAS_ORTI))
  osEE_orti_trace_service_entry(p_curr_ccb, OSServiceId_SetEvent);
#endif /* OSEE_HAS_CHECKS elif OSEE_HAS_ORTI */
#endif /* OSEE_HAS_CHECKS || OSEE_HAS_ERRORHOOK || OSEE_HAS_ORTI */
  if (!osEE_is_valid_tid(p_kdb, TaskID)) {
    ev = E_OS_ID;
  } else {
    P2VAR(OsEE_SN, AUTOMATIC, OS_APPL_DATA)
      p_sn;
    CONSTP2VAR(OsEE_TDB, AUTOMATIC, OS_APPL_DATA)
      p_tdb_waking_up = (*p_kdb->p_tdb_ptr_array)[TaskID];
    CONST(OsEE_reg, AUTOMATIC)
      flags = osEE_begin_primitive();

    p_sn = osEE_task_event_set_mask(p_tdb_waking_up, Mask, &ev);

    if (p_sn != NULL) {
      /* Release the TASK (and the SN) */
      if (osEE_scheduler_task_unblocked(p_kdb, p_sn))
      {
        (void)osEE_scheduler_task_preemption_point(p_kdb);
      }
    }
    osEE_end_primitive(flags);
  }

#if (defined(OSEE_HAS_ERRORHOOK))
  if (ev != E_OK) {
    VAR(OsEE_api_param, AUTOMATIC)
      param;
    CONST(OsEE_reg, AUTOMATIC)
      flags = osEE_begin_primitive();
    osEE_set_service_id(p_curr_ccb, OSServiceId_SetEvent);
    param.num_param = TaskID;
    osEE_set_api_param1(p_curr_ccb, param);
    param.num_param = Mask;
    osEE_set_api_param2(p_curr_ccb, param);
    osEE_call_error_hook(p_curr_ccb, ev);
    osEE_end_primitive(flags);
  }
#endif /* OSEE_HAS_ERRORHOOK */
#if (defined(OSEE_HAS_ORTI))
  osEE_orti_trace_service_exit(p_curr_ccb, OSServiceId_SetEvent);
#endif /* OSEE_HAS_ORTI */

  return ev;
}

FUNC(StatusType, OS_CODE)
  GetEvent
(
  VAR(TaskType, AUTOMATIC)          TaskID,
  VAR(EventMaskRefType, AUTOMATIC)  Event
)
{
  VAR(StatusType, AUTOMATIC)  ev;
  CONSTP2VAR(OsEE_KDB, AUTOMATIC, OS_APPL_DATA)
    p_kdb       = osEE_get_kernel();
#if (defined(OSEE_HAS_CHECKS)) || (defined(OSEE_HAS_ERRORHOOK)) ||\
    (defined(OSEE_HAS_ORTI))
  CONSTP2VAR(OsEE_CDB, AUTOMATIC, OS_APPL_DATA)
    p_cdb       = osEE_get_curr_core();
  CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA)
    p_ccb       = p_cdb->p_ccb;
#if (defined(OSEE_SERVICE_PROTECTION))
  CONST(OsEE_os_context, AUTOMATIC)
    os_context  = p_ccb->os_context;
#endif /* OSEE_SERVICE_PROTECTION */
#if (defined(OSEE_HAS_CHECKS))
  CONSTP2VAR(OsEE_TDB, AUTOMATIC, OS_APPL_DATA)
    p_curr      = p_ccb->p_curr;

  osEE_orti_trace_service_entry(p_ccb, OSServiceId_GetEvent);

  /*  [OS_SWS_093]: If interrupts are disabled/suspended by a Task/OsIsr and
   *    the Task/OsIsr calls any OS service (excluding the interrupt services)
   *    then the Operating System shall ignore the service AND shall return */
  /*  [OS_SWS_088]: If an OS-Application makes a service call from the wrong
   *    context AND is currently not inside a Category 1 ISR the Operating
   *    System module shall not perform the requested action
   *    (the service call shall have no effect), and return E_OS_CALLEVEL
   *    (see [12], section 13.1) or the "invalid value" of  the service.
   *    (BSW11009, BSW11013) */
  if (osEE_check_disableint(p_ccb)) {
    ev = E_OS_DISABLEDINT;
  } else
  if ((p_curr->task_type > OSEE_TASK_TYPE_ISR2)
#if (!defined(OSEE_SERVICE_PROTECTION))
  )
#else
    || ((os_context > OSEE_ERRORHOOK_CTX) &&
        (os_context != OSEE_PRETASKHOOK_CTX) &&
        (os_context != OSEE_POSTTASKHOOK_CTX))
  )
#endif /* OSEE_SERVICE_PROTECTION */
  {
    ev = E_OS_CALLEVEL;
  } else
#elif (defined(OSEE_HAS_ORTI))
  osEE_orti_trace_service_entry(p_ccb, OSServiceId_GetEvent);
#endif /* OSEE_HAS_CHECKS elif OSEE_HAS_ORTI */
#endif /* OSEE_HAS_CHECKS || OSEE_HAS_ERRORHOOK || OSEE_HAS_ORTI */
  if (!osEE_is_valid_tid(p_kdb, TaskID)) {
    ev = E_OS_ID;
  } else
  {
    CONSTP2VAR(OsEE_TDB, AUTOMATIC, OS_APPL_DATA)
      p_tdb_event = (*p_kdb->p_tdb_ptr_array)[TaskID];
    CONSTP2VAR(OsEE_TCB, AUTOMATIC, OS_APPL_DATA)
      p_tcb_event = p_tdb_event->p_tcb;
    /* XXX: We will accept an harmless race condition here for TASKs that want
     *      read events of TASKs allocated in other cores */
#if (defined(OSEE_HAS_CHECKS))
    if (p_tdb_event->task_type != OSEE_TASK_TYPE_EXTENDED) {
      ev = E_OS_ACCESS;
    } else
    if (p_tcb_event->status == OSEE_TASK_SUSPENDED) {
      ev = E_OS_STATE;
    } else
#endif /* OSEE_HAS_CHECKS */
    if (Event == NULL) {
      ev = E_OS_PARAM_POINTER;
    } else
    {
      (*Event) = p_tcb_event->event_mask;

      ev = E_OK;
    }
  }

#if (defined(OSEE_HAS_ERRORHOOK))
  if (ev != E_OK) {
    VAR(OsEE_api_param, AUTOMATIC)
      param;
    CONST(OsEE_reg, AUTOMATIC)
      flags = osEE_begin_primitive();
    osEE_set_service_id(p_ccb, OSServiceId_GetEvent);
    param.num_param = TaskID;
    osEE_set_api_param1(p_ccb, param);
    param.p_param   = Event;
    osEE_set_api_param2(p_ccb, param);
    osEE_call_error_hook(p_ccb, ev);
    osEE_end_primitive(flags);
  }
#endif /* OSEE_HAS_ERRORHOOK */
#if (defined(OSEE_HAS_ORTI))
  osEE_orti_trace_service_exit(p_ccb, OSServiceId_GetEvent);
#endif /* OSEE_HAS_ORTI */

  return ev;
}

FUNC(StatusType, OS_CODE)
  ClearEvent
(
  VAR(EventMaskType, AUTOMATIC) Mask
)
{
  VAR(StatusType, AUTOMATIC)  ev;
  CONSTP2VAR(OsEE_CDB, AUTOMATIC, OS_APPL_DATA)
    p_cdb       = osEE_get_curr_core();
  CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA)
    p_ccb       = p_cdb->p_ccb;
  CONSTP2VAR(OsEE_TDB, AUTOMATIC, OS_APPL_DATA)
    p_curr      = p_ccb->p_curr;
  CONSTP2VAR(OsEE_TCB, AUTOMATIC, OS_APPL_DATA)
    p_curr_tcb  = p_curr->p_tcb;

  osEE_orti_trace_service_entry(p_ccb, OSServiceId_ClearEvent);

#if (defined(OSEE_HAS_CHECKS))
  /*  [OS_SWS_093]: If interrupts are disabled/suspended by a Task/OsIsr and
   *    the Task/OsIsr calls any OS service (excluding the interrupt services)
   *    then the Operating System shall ignore the service AND shall return */
  /*  [OS_SWS_088]: If an OS-Application makes a service call from the wrong
   *    context AND is currently not inside a Category 1 ISR the Operating
   *    System module shall not perform the requested action
   *    (the service call shall have no effect), and return E_OS_CALLEVEL
   *    (see [12], section 13.1) or the "invalid value" of  the service.
   *    (BSW11009, BSW11013) */
  if (osEE_check_disableint(p_ccb)) {
    ev = E_OS_DISABLEDINT;
  } else
  if ((p_curr->task_type >= OSEE_TASK_TYPE_ISR2)
#if (!defined(OSEE_SERVICE_PROTECTION))
  )
#else
    || (p_ccb->os_context > OSEE_TASK_CTX)
  )
#endif /* OSEE_SERVICE_PROTECTION */
  {
    ev = E_OS_CALLEVEL;
  } else
  if (p_curr->task_type != OSEE_TASK_TYPE_EXTENDED) {
    ev = E_OS_ACCESS;
  } else
#endif /* OSEE_HAS_CHECKS */
  {
    /* clear the event */
    p_curr_tcb->event_mask &= ~Mask;

    ev = E_OK;
  }
#if (defined(OSEE_HAS_ERRORHOOK))
  if (ev != E_OK) {
    VAR(OsEE_api_param, AUTOMATIC)
      param;
    CONST(OsEE_reg, AUTOMATIC)
      flags = osEE_begin_primitive();
    osEE_set_service_id(p_ccb, OSServiceId_ClearEvent);
    param.num_param = Mask;
    osEE_set_api_param1(p_ccb, param);
    osEE_call_error_hook(p_ccb, ev);
    osEE_end_primitive(flags);
  }
#endif /* OSEE_HAS_ERRORHOOK */

  osEE_orti_trace_service_exit(p_ccb, OSServiceId_ClearEvent);

  return ev;
}
#endif /* OSEE_HAS_EVENTS */

#if (defined(OSEE_USEPARAMETERACCESS))
FUNC(OSServiceIdType, OS_CODE)
  osEE_get_service_id
(
  void
)
{
#if (defined(OSEE_HAS_ORTI))
  return osEE_get_curr_core()->p_ccb->service_id & (~((OSServiceIdType)0x1U));
#else
  return osEE_get_curr_core()->p_ccb->service_id;
#endif /* OSEE_HAS_ORTI */
}

FUNC(OsEE_api_param, OS_CODE)
  osEE_get_api_param1
(
  void
)
{
  return osEE_get_curr_core()->p_ccb->api_param1;
}
FUNC(OsEE_api_param, OS_CODE)
  osEE_get_api_param2
(
  void
)
{
  return osEE_get_curr_core()->p_ccb->api_param2;
}
FUNC(OsEE_api_param, OS_CODE)
  osEE_get_api_param3
(
  void
)
{
  return osEE_get_curr_core()->p_ccb->api_param3;
}
#endif /* OSEE_USEPARAMETERACCESS */

#if (defined(OSEE_HAS_COUNTERS))
FUNC(StatusType, OS_CODE)
  GetCounterValue
(
  VAR(CounterType, AUTOMATIC) CounterID,
  VAR(TickRefType, AUTOMATIC) Value
)
{
  VAR(StatusType, AUTOMATIC)  ev;
  CONSTP2VAR(OsEE_KDB, AUTOMATIC, OS_APPL_DATA)
    p_kdb = osEE_get_kernel();
#if (defined(OSEE_HAS_ORTI))
  CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA)
    p_ccb = osEE_get_curr_core()->p_ccb;
  osEE_orti_trace_service_entry(p_ccb, OSServiceId_GetCounterValue);
#endif /* OSEE_HAS_ORTI */
/* [SWS_Os_00376] If the input parameter <CounterID> in a call of
    GetCounterValue() is not valid, GetCounterValue() shall return E_OS_ID. */
  if (!osEE_is_valid_counter_id(p_kdb, CounterID)) {
    ev = E_OS_ID;
  } else
#if (defined(OSEE_HAS_CHECKS))
  if (Value == NULL) {
    ev = E_OS_PARAM_POINTER;
  } else
#endif /* OSEE_HAS_CHECKS */
  {
    CONSTP2VAR(OsEE_CounterDB, AUTOMATIC, OS_APPL_DATA)
      p_counter_db = (*p_kdb->p_counter_ptr_array)[CounterID];
/* [SWS_Os_00589] All functions that are not allowed to operate cross core
    shall return E_OS_CORE in extended status if called with parameters that
    require a cross core operation. (SRS_Os_80013) */
#if (!defined(OSEE_SINGLECORE))
    if (p_counter_db->core_id != osEE_get_curr_core_id()) {
      ev = E_OS_CORE;
    } else
#endif /* !OSEE_SINGLECORE */
    {
/* [SWS_Os_00377] If the input parameter <CounterID> in a call of
    GetCounterValue() is valid, GetCounterValue() shall return the current tick
    value of the counter via <Value> and return E_OK. (SRS_Frt_00033) */
/* [SWS_Os_00531] Caveats of GetCounterValue(): Note that for counters of
    OsCounterType = HARDWARE the real timer value
   (the – possibly adjusted – hardware value, see SWS_Os_00384) is returned,
   whereas for counters of OsCounterType = SOFTWARE the current "software"
   tick value is returned. */
/* [SWS_Os_00384] The Operating System module shall adjust the read out values
    of hardware timers (which drive counters) in such that the lowest value is
    zero and consecutive reads return an increasing count value until the timer
    wraps at its modulus. (SRS_Frt_00030, SRS_Frt_00031) */
/* EG  TODO: Add support for HARDWARE counters */

      (*Value) = p_counter_db->p_counter_cb->value;

      ev = E_OK;
    }
  }

#if (defined(OSEE_HAS_ERRORHOOK))
  if (ev != E_OK) {
    VAR(OsEE_api_param, AUTOMATIC)
      param;
#if (!defined(OSEE_HAS_ORTI))
    CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA)
      p_ccb = osEE_get_curr_core()->p_ccb;
#endif /* !OSEE_HAS_ORTI */
    CONST(OsEE_reg, AUTOMATIC)
      flags = osEE_begin_primitive();
    osEE_set_service_id(p_ccb, OSServiceId_GetCounterValue);
    param.num_param = CounterID;
    osEE_set_api_param1(p_ccb, param);
    param.p_param   = Value;
    osEE_set_api_param2(p_ccb, param);
    osEE_call_error_hook(p_ccb, ev);
    osEE_end_primitive(flags);
  }
#endif /* OSEE_HAS_ERRORHOOK */
#if (defined(OSEE_HAS_ORTI))
  osEE_orti_trace_service_exit(p_ccb, OSServiceId_GetCounterValue);
#endif /* OSEE_HAS_ORTI */
  return ev;
}

FUNC(StatusType, OS_CODE)
  GetElapsedValue
(
  VAR(CounterType, AUTOMATIC) CounterID,
  VAR(TickRefType, AUTOMATIC) Value,
  VAR(TickRefType, AUTOMATIC) ElapsedValue
)
{
  VAR(StatusType, AUTOMATIC)  ev;
  CONSTP2VAR(OsEE_KDB, AUTOMATIC, OS_APPL_DATA)
    p_kdb = osEE_get_kernel();
#if (defined(OSEE_HAS_ORTI))
  CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA)
    p_ccb = osEE_get_curr_core()->p_ccb;
  osEE_orti_trace_service_entry(p_ccb, OSServiceId_GetElapsedValue);
#endif /* OSEE_HAS_ORTI */

/* [SWS_Os_00381] If the input parameter <CounterID> in a call of
    GetElapsedValue() is not valid GetElapsedValue() shall return E_OS_ID. */
  if (!osEE_is_valid_counter_id(p_kdb, CounterID)) {
    ev = E_OS_ID;
  } else
#if (defined(OSEE_HAS_CHECKS))
  if ((Value == NULL) || (ElapsedValue == NULL)) {
    ev = E_OS_PARAM_POINTER;
  } else
#endif /* OSEE_HAS_CHECKS */
  {
    CONSTP2VAR(OsEE_CounterDB, AUTOMATIC, OS_APPL_DATA)
      p_counter_db = (*p_kdb->p_counter_ptr_array)[CounterID];
    CONST(TickType, AUTOMATIC)
      local_value = (*Value);

/* [SWS_Os_00589] All functions that are not allowed to operate cross core
    shall return E_OS_CORE in extended status if called with parameters that
    require a cross core operation. (SRS_Os_80013) */
#if (!defined(OSEE_SINGLECORE))
    if (p_counter_db->core_id != osEE_get_curr_core_id()) {
      ev = E_OS_CORE;
    } else
#endif /* !OSEE_SINGLECORE */
#if (defined(OSEE_HAS_CHECKS))
/* [SWS_Os_00391] If the <Value> in a call of GetElapsedValue() is larger than
    the max allowed value of the <CounterID>, GetElapsedValue() shall return
    E_OS_VALUE. */
    if (local_value > p_counter_db->info.maxallowedvalue) {
      ev = E_OS_VALUE;
    } else
#endif /* OSEE_HAS_CHECKS */
    {
/* [SWS_Os_00382] If the input parameters in a call of GetElapsedValue()
     are valid, GetElapsedValue() shall return the number of elapsed ticks
     since the given <Value> value via <ElapsedValue> and shall return
     E_OK. (SRS_Frt_00034) */
      CONST(TickType, AUTOMATIC)
        local_curr_value = p_counter_db->p_counter_cb->value;

/* [SWS_Os_00533] Caveats of GetElapsedValue(): If the timer already passed the
    <Value> value a second (or multiple) time, the result returned is wrong.
    The reason is that the service can not detect such a relative overflow. */
/* EG  TODO: Add support for HARDWARE counters */
      (*ElapsedValue) = (local_curr_value >= local_value)?
        /* Timer did not pass the <value> yet */
        (local_curr_value - local_value):
        /* Timer already passed the <value> */
        ((p_counter_db->info.maxallowedvalue -
          (local_value - local_curr_value)) + 1U);

/* [SWS_Os_00460] GetElapsedValue() shall return the current tick value of the
    counter in the <Value> parameter. */
      (*Value) = local_curr_value;

      ev = E_OK;
    }
  }

#if (defined(OSEE_HAS_ERRORHOOK))
  if (ev != E_OK) {
    VAR(OsEE_api_param, AUTOMATIC)
      param;
#if (!defined(OSEE_HAS_ORTI))
    CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA)
      p_ccb = osEE_get_curr_core()->p_ccb;
#endif /* !OSEE_HAS_ORTI */
    CONST(OsEE_reg, AUTOMATIC)
      flags = osEE_begin_primitive();
    osEE_set_service_id(p_ccb, OSServiceId_GetElapsedValue);
    param.num_param = CounterID;
    osEE_set_api_param1(p_ccb, param);
    param.p_param   = Value;
    osEE_set_api_param2(p_ccb, param);
    param.p_param   = ElapsedValue;
    osEE_set_api_param3(p_ccb, param);
    osEE_call_error_hook(p_ccb, ev);
    osEE_end_primitive(flags);
  }
#endif /* OSEE_HAS_ERRORHOOK */
#if (defined(OSEE_HAS_ORTI))
  osEE_orti_trace_service_exit(p_ccb, OSServiceId_GetElapsedValue);
#endif /* OSEE_HAS_ORTI */

  return ev;
}


FUNC(StatusType, OS_CODE)
  IncrementCounter
(
  VAR(CounterType, AUTOMATIC) CounterID
)
{
  VAR(StatusType, AUTOMATIC)  ev;
  CONSTP2VAR(OsEE_KDB, AUTOMATIC, OS_APPL_DATA)
    p_kdb = osEE_get_kernel();
#if (defined(OSEE_HAS_ORTI))
  CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA)
    p_ccb = osEE_get_curr_core()->p_ccb;
  osEE_orti_trace_service_entry(p_ccb, OSServiceId_IncrementCounter);
#endif /* OSEE_HAS_ORTI */

/* [SWS_Os_00285] If the input parameter <CounterID> in a call of
    IncrementCounter() is not valid OR the counter is a hardware counter,
    IncrementCounter() shall return E_OS_ID. */
  if (!osEE_is_valid_counter_id(p_kdb, CounterID)) {
    ev = E_OS_ID;
  } else
  {
    CONSTP2VAR(OsEE_CounterDB, AUTOMATIC, OS_APPL_DATA)
      p_counter_db = (*p_kdb->p_counter_ptr_array)[CounterID];
/* [SWS_Os_00589] All functions that are not allowed to operate cross core
    shall return E_OS_CORE in extended status if called with parameters that
    require a cross core operation. (SRS_Os_80013) */
#if (!defined(OSEE_SINGLECORE))
    if (p_counter_db->core_id != osEE_get_curr_core_id()) {
      ev = E_OS_CORE;
    } else
#endif /* !OSEE_SINGLECORE */
    {
/* [SWS_Os_00286] If the input parameter of IncrementCounter() is valid,
    IncrementCounter() shall increment the counter <CounterID> by one
    (if any alarm connected to this counter expires, the given action,
    e.g. task activation, is done) and shall return E_OK. (SRS_Os_11020) */
/* [SWS_Os_00321] If in a call of IncrementCounter() an error happens during
    the execution of an alarm action,
    e.g. E_OS_LIMIT caused by a task activation, IncrementCounter() shall call
    the error hook(s), but the IncrementCounter() service itself shall
    return E_OK. */
      CONST(OsEE_reg, AUTOMATIC) flags = osEE_begin_primitive();

/* N.B. Multi-core critical sections are handled inside
   TODO: Pass flags to osEE_counter_increment so it could re-enable
         interrupts/lower IPL outside critical sections. */
      osEE_counter_increment(p_counter_db);

/* [SWS_Os_00529] Caveats of IncrementCounter(): If called from a task,
    rescheduling may take place. */
      if (osEE_get_curr_task()->task_type <= OSEE_TASK_TYPE_EXTENDED) {
        (void)osEE_scheduler_task_preemption_point(p_kdb);
      }

      osEE_end_primitive(flags);

      ev = E_OK;
    }
  }

#if (defined(OSEE_HAS_ERRORHOOK))
  if (ev != E_OK) {
    VAR(OsEE_api_param, AUTOMATIC)
      param;
#if (!defined(OSEE_HAS_ORTI))
    CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA)
      p_ccb = osEE_get_curr_core()->p_ccb;
#endif /* !OSEE_HAS_ORTI */
    CONST(OsEE_reg, AUTOMATIC)
      flags = osEE_begin_primitive();
    osEE_set_service_id(p_ccb, OSServiceId_IncrementCounter);
    param.num_param = CounterID;
    osEE_set_api_param1(p_ccb, param);
    osEE_call_error_hook(p_ccb, ev);
    osEE_end_primitive(flags);
  }
#endif /* OSEE_HAS_ERRORHOOK */
#if (defined(OSEE_HAS_ORTI))
  osEE_orti_trace_service_exit(p_ccb, OSServiceId_IncrementCounter);
#endif /* OSEE_HAS_ORTI */

  return ev;
}

#endif /* OSEE_HAS_COUNTERS */

FUNC(ISRType, OS_CODE)
  GetISRID
(
  void
)
{
  VAR(ISRType, AUTOMATIC) isr_id;
  CONSTP2CONST(OsEE_TDB, AUTOMATIC, OS_APPL_DATA)
    p_tdb = osEE_get_curr_task();

  if (p_tdb->task_type == OSEE_TASK_TYPE_ISR2) {
    isr_id = p_tdb->tid;
  } else {
    isr_id = INVALID_ISR;
  }

  return isr_id;
}

#if (!defined(OSEE_SINGLECORE))

/* FIXME: from specification return value should be uint32 */
FUNC(CoreIdType, OS_CODE)
  GetNumberOfActivatedCores
(
  void
)
{
  /* [SWS_Os_00673] The return value of GetNumberOfActivatedCores shall be less
      or equal to the configured value of "OsNumberOfCores". (SRS_Os_80001) */
  return osEE_get_kernel()->p_kcb->ar_num_core_started;
}

FUNC(void, OS_CODE)
  StartCore
(
  VAR(CoreIdType, AUTOMATIC)                  CoreID,
  P2VAR(StatusType, AUTOMATIC, OS_APPL_DATA)  Status
)
{
  /* Error Value */
  VAR(StatusType, AUTOMATIC) ev;

  CONSTP2VAR(OsEE_CDB, AUTOMATIC, OS_APPL_DATA)
    p_cdb       = osEE_get_curr_core();
  CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA)
    p_ccb       = p_cdb->p_ccb;
  CONST(OsEE_reg, AUTOMATIC)
    flags = osEE_begin_primitive();
  CONSTP2VAR(OsEE_KDB, AUTOMATIC, OS_APPL_DATA)
    p_kdb = osEE_lock_and_get_kernel();
  CONSTP2VAR(OsEE_KCB, AUTOMATIC, OS_APPL_DATA)
    p_kcb = p_kdb->p_kcb;
  /* Variable introduced to meet MISRA 12.1 in the next else if statement */
  CONST(OSEE_CORE_MASK_TYPE, AUTOMATIC)
    ar_core_mask = p_kcb->ar_core_mask;
  CONST(OSEE_CORE_MASK_TYPE, AUTOMATIC)
    core_id_mask =  (1U << CoreID);

  if ((core_id_mask & OSEE_CORE_ID_VALID_MASK) == 0U) {
      ev = E_OS_ID;
  } else if (p_ccb->os_status != OSEE_KERNEL_INITIALIZED) {
    /* [SWS_Os_00606] The AUTOSAR specification does not support the activation
        of AUTOSAR cores after calling StartOS on that core.
        If StartCore is called after StartOS it shall return with E_OS_ACCESS
        in extended status. (SRS_Os_80001) */
    /* [SWS_Os_00678] Calls to the StartCore function after StartOS()
        shall return with E_OS_ACCESS and the core shall not be started.
        (SRS_Os_80006, SRS_Os_80026, SRS_Os_80027) */
      ev = E_OS_ACCESS;
  } else if (((ar_core_mask | p_kcb->not_ar_core_mask) & core_id_mask) != 0U) {
    /* [SWS_Os_00679] If the parameter CoreIDs refers to a core that was
        already started by the function StartCore the related core is ignored
        and E_OS_STATE shall be returned.
        (SRS_Os_80006, SRS_Os_80026, SRS_Os_80027) */
    /* [SWS_Os_00680] If the parameter CoreID refers to a core that was already
        started by the function StartNonAutosarCore the related core is ignored
        and E_OS_STATE shall be returned.
        (SRS_Os_80006, SRS_Os_80026, SRS_Os_80027) */
    ev = E_OS_STATE;
  } else {
    /* Really start the core if we are not in MASTER core */
    if (CoreID != OS_CORE_ID_MASTER) {
      /* [SWS_Os_00677] The function StartCore shall start one core that shall
          run under the control of the AUTOSAR OS.
          (SRS_Os_80006, SRS_Os_80026, SRS_Os_80027) */
      /* Flag that core is started as Autosar core */
      p_kcb->ar_core_mask |= core_id_mask;
      /* Increment the Autosar Cores counter */
      ++p_kcb->ar_num_core_started;

      osEE_hal_start_core(CoreID);
    }

    ev = E_OK;
  }

  /* Restore the initial conditions */
  osEE_unlock_kernel();

  osEE_end_primitive(flags);

  /* [SWS_Os_00681] There is no call to the ErrorHook() if an error occurs
       during StartCore(); (SRS_Os_80006, SRS_Os_80026, SRS_Os_80027) */
  if (Status != NULL) {
    *Status = ev;
  }

  return;
}

FUNC(void, OS_CODE)
  StartNonAutosarCore
(
  VAR(CoreIdType, AUTOMATIC)                  CoreID,
  P2VAR(StatusType, AUTOMATIC, OS_APPL_DATA)  Status
)
{
  /* Error Value */
  VAR(StatusType, AUTOMATIC) ev;

  CONST(OsEE_reg, AUTOMATIC)
    flags = osEE_begin_primitive();
  CONSTP2VAR(OsEE_KDB, AUTOMATIC, OS_APPL_DATA)
    p_kdb = osEE_lock_and_get_kernel();
  CONSTP2VAR(OsEE_KCB, AUTOMATIC, OS_APPL_DATA)
    p_kcb = p_kdb->p_kcb;
  /* Variable introduced to meet MISRA 12.1 in the next else if statement */
  CONST(OSEE_CORE_MASK_TYPE, AUTOMATIC)
    ar_core_mask = p_kcb->ar_core_mask;
  CONST(OSEE_CORE_MASK_TYPE, AUTOMATIC)
    core_id_mask =  (1U << CoreID);

  if ((core_id_mask & OSEE_CORE_ID_VALID_MASK) == 0U) {
    /* [SWS_Os_00685] If the parameter CoreID refers to an unknown core the
        function StartNonAutosarCore has no effect and sets "Status" to E_OS_ID.
        (SRS_Os_80006, SRS_Os_80026, SRS_Os_80027) */
    ev = E_OS_ID;
  } else if (((ar_core_mask | p_kcb->not_ar_core_mask) & core_id_mask) != 0U) {
    /* [SWS_Os_00680] If the parameter CoreID refers to a core that was already
        started by the function StartNonAutosarCore the related core is ignored
        and E_OS_STATE shall be returned.
        (SRS_Os_80006, SRS_Os_80026, SRS_Os_80027) */
    ev = E_OS_STATE;
  } else {
    /* Really start the core if we are not in MASTER core */
    if (CoreID != OS_CORE_ID_MASTER) {
    /* [SWS_Os_00683] The function StartNonAutosarCore shall start a core that
        is not controlled by the AUTOSAR OS.
        (SRS_Os_80006, SRS_Os_80026, SRS_Os_80027) */
      /* Flag that core is started as non Autosar core */
      p_kcb->not_ar_core_mask |= core_id_mask;
      osEE_hal_start_core(CoreID);
    }

    ev = E_OK;
  }

  /* Restore the initial conditions */
  osEE_unlock_kernel();

  osEE_end_primitive(flags);

  if (Status != NULL) {
    *Status = ev;
  }

  return;

}
#endif /* !OSEE_SINGLECORE */