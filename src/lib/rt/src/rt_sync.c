/* 
 * Proview   Open Source Process Control.
 * Copyright (C) 2005-2016 SSAB EMEA AB.
 *
 * This file is part of Proview.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation, either version 2 of 
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with Proview. If not, see <http://www.gnu.org/licenses/>
 *
 * Linking Proview statically or dynamically with other modules is
 * making a combined work based on Proview. Thus, the terms and 
 * conditions of the GNU General Public License cover the whole 
 * combination.
 *
 * In addition, as a special exception, the copyright holders of
 * Proview give you permission to, from the build function in the
 * Proview Configurator, combine Proview with modules generated by the
 * Proview PLC Editor to a PLC program, regardless of the license
 * terms of these modules. You may copy and distribute the resulting
 * combined work under the terms of your choice, provided that every 
 * copy of the combined work is accompanied by a complete copy of 
 * the source code of Proview (the version used to produce the 
 * combined work), being distributed under the terms of the GNU 
 * General Public License plus this exception.
 */

#ifdef	OS_ELN
# include $vaxelnc
# include $kernelmsg
# include stdio
# include stdlib
# include string
#else
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
#endif

#ifdef OS_VMS
# include <starlet.h>
# include <lib$routines.h>
# include <tis.h>
#endif

#include "pwr.h"
#include "co_errno.h"
#include "rt_errh.h"
#include "rt_sync.h"
#include "rt_sync_msg.h"


pwr_tStatus
sync_CondInit (
  thread_sCond	*cp
)
{

#if defined OS_ELN

  pwr_tStatus sts;

  ker$create_event(&sts, cp, EVENT$CLEARED);

  return sts;

#elif defined OS_LYNX && defined PWR_LYNX_30

  return errno_Pstatus(pthread_cond_init(&cp->c, NULL));

#elif defined OS_POSIX

  return errno_Status(pthread_cond_init(&cp->c, NULL));

#elif defined OS_VMS

  return errno_Status(tis_cond_init(&cp->c));

#else
# error Not defined for this platform !
#endif
}

pwr_tStatus
sync_CondSignal (
  thread_sCond	*cp
)
{

#if defined OS_ELN

  pwr_tStatus sts = SYNC__SUCCESS;

  ker$signal(&sts, *cp);

  return sts;

#elif defined OS_LYNX && defined PWR_LYNX_30
  cp->f = 1;
  return errno_Pstatus(pthread_cond_signal(&cp->c));

#elif defined OS_POSIX
  cp->f = 1;
  return errno_Status(pthread_cond_signal(&cp->c));

#elif defined OS_VMS

  return errno_Status(tis_cond_signal(&cp->c));

#else
# error Not defined for this platform !
#endif
}

pwr_tStatus
sync_MutexInit (
  thread_sMutex		*mp
)
{

#if defined OS_ELN

  pwr_tStatus sts = SYNC__SUCCESS;

  ELN$CREATE_MUTEX(*mp, &sts);

  return sts;

#elif defined OS_LYNX && defined PWR_LYNX_30

  return errno_Pstatus(pthread_mutex_init(mp, NULL));

#elif defined OS_POSIX

  return errno_Status(pthread_mutex_init(mp, NULL));

#elif defined OS_VMS

  return errno_Status(tis_mutex_init(mp));

#else
# error Not defined for this platform !
#endif
}

pwr_tStatus
sync_MutexLock (
  thread_sMutex		*mp
)
{

#if defined OS_ELN

  ELN$LOCK_MUTEX(*mp);
  return SYNC__SUCCESS;

#elif defined OS_LYNX && defined PWR_LYNX_30

  return errno_Pstatus(pthread_mutex_lock(mp));

#elif defined OS_POSIX

  return errno_Status(pthread_mutex_lock(mp));

#elif defined OS_VMS

  return errno_Status(tis_mutex_lock(mp));

#else
# error Not defined for this platform !
#endif

}

pwr_tStatus
sync_MutexUnlock (
  thread_sMutex		*mp
)
{

#if defined OS_ELN

  ELN$UNLOCK_MUTEX(*mp);
  return SYNC__SUCCESS;

#elif defined OS_LYNX && defined PWR_LYNX_30

  return errno_Pstatus(pthread_mutex_unlock(mp));

#elif defined OS_POSIX

  return errno_Status(pthread_mutex_unlock(mp));

#elif defined OS_VMS

  return errno_Status(tis_mutex_unlock(mp));

#else
# error Not defined for this platform !
#endif

}
