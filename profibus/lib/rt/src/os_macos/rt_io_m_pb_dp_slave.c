/* 
 * Proview   $Id: rt_io_m_pb_dp_slave.c,v 1.12 2008-02-29 13:15:42 claes Exp $
 * Copyright (C) 2005 SSAB Oxel�sund AB.
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
 * along with the program, if not, write to the Free Software 
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* rt_io_m_pb_dp_slave.c -- io methods for a profibus DP slave */

#pragma pack(1)

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/ioctl.h>

#include "rt_io_pb_locals.h"

#include "pwr.h"
#include "co_cdh.h"
#include "pwr_baseclasses.h"
#include "pwr_profibusclasses.h"
#include "rt_gdh.h"
#include "rt_io_base.h"
#include "rt_io_bus.h"
#include "rt_io_msg.h"
#include "rt_errh.h"
#include "co_cdh.h"
#include "rt_io_profiboard.h"
#include "rt_pb_msg.h"

/* Check if channel should be fetched from diagnostic area, 
   i.e. channel name starts with "Diag_" */


/*----------------------------------------------------------------------------*\
   Init method for the Pb DP slave 
\*----------------------------------------------------------------------------*/

static pwr_tStatus IoRackInit (
  io_tCtx	ctx,
  io_sAgent	*ap,
  io_sRack	*rp
) 
{
  io_sCardLocal *local_card;
  io_sCard *cardp;
  short input_counter;
  short output_counter;
  pwr_sClass_Pb_DP_Slave *op;
  pwr_sClass_Pb_Di *dip;
  pwr_sClass_Pb_Do *dop;
  pwr_sClass_Pb_Ai *aip;
  pwr_sClass_Pb_Ao *aop;
  pwr_sClass_Pb_Ii *iip;
  pwr_sClass_Pb_Io *iop;
  pwr_sClass_Pb_Module *mp;
  char name[196];
  pwr_tStatus sts;
  pwr_tCid cid;
  
  io_sChannel *chanp;
  int i, latent_input_count, latent_output_count;
  pwr_tInt32 chan_size;
  pwr_sClass_ChanDi *chan_di;
  pwr_sClass_ChanDo *chan_do;
  pwr_sClass_ChanAi *chan_ai;
  pwr_sClass_ChanAit *chan_ait;
  pwr_sClass_ChanIi *chan_ii;
  pwr_sClass_ChanAo *chan_ao;
  pwr_sClass_ChanIo *chan_io;

  sts = gdh_ObjidToName(rp->Objid, (char *) &name, sizeof(name), cdh_mNName);
  errh_Info( "Init of Profibus DP Slave and Modules %s", name);

  op = (pwr_sClass_Pb_DP_Slave *) rp->op;
  
  // Do configuration check and initialize modules.

  cardp = rp->cardlist;
  input_counter = 0;
  output_counter = 0;
  op->NumberModules = 0;

  latent_input_count = 0;
  latent_output_count = 0;

  while(cardp) {
    local_card = calloc(1, sizeof(*local_card));
    cardp->Local = local_card;
    local_card->input_area = (void *) &(op->Inputs);
    local_card->output_area = (void *) &(op->Outputs);

    /* From v4.1.3 we can have subclasses, find the super class */
    
    cid = cardp->Class;
    while ( ODD( gdh_GetSuperClass( cid, &cid, cardp->Objid))) ;

    switch (cid) {

      /* Old style configuring with Pb_xx objects. Still here for combatibility reasons. 
         New systems (from v4.1.3) should be build with Pb_Module objects or subclasses */
	 
      case pwr_cClass_Pb_Di:
        dip = (pwr_sClass_Pb_Di *) cardp->op;
        dip->OffsetInputs = input_counter;
        dip->BytesOfInput = dip->NumberOfChannels / 8;
        input_counter += dip->BytesOfInput;
        dip->Status = PB_MODULE_STATE_OPERATE;
        break;

      case pwr_cClass_Pb_Do:
        dop = (pwr_sClass_Pb_Do *) cardp->op;
        dop->OffsetOutputs = output_counter;
        dop->BytesOfOutput = dop->NumberOfChannels / 8;
        output_counter += dop->BytesOfOutput;
        dop->Status = PB_MODULE_STATE_OPERATE;
        break;

      case pwr_cClass_Pb_Ai:
        aip = (pwr_sClass_Pb_Ai *) cardp->op;
        aip->OffsetInputs = input_counter;
        aip->BytesOfInput = aip->NumberOfChannels * aip->BytesPerChannel;
        input_counter += aip->BytesOfInput;
        aip->Status = PB_MODULE_STATE_OPERATE;
        break;

      case pwr_cClass_Pb_Ao:
        aop = (pwr_sClass_Pb_Ao *) cardp->op;
        aop->OffsetOutputs = output_counter;
        aop->BytesOfOutput = aop->NumberOfChannels * aop->BytesPerChannel;
        output_counter += aop->BytesOfOutput;
        aop->Status = PB_MODULE_STATE_OPERATE;
        break;

      case pwr_cClass_Pb_Ii:
        iip = (pwr_sClass_Pb_Ii *) cardp->op;
        iip->OffsetInputs = input_counter;
        iip->BytesOfInput = iip->NumberOfChannels * iip->BytesPerChannel;
        input_counter += iip->BytesOfInput;
        iip->Status = PB_MODULE_STATE_OPERATE;
        break;

      case pwr_cClass_Pb_Io:
        iop = (pwr_sClass_Pb_Io *) cardp->op;
        iop->OffsetOutputs = output_counter;
        iop->BytesOfOutput = iop->NumberOfChannels * iop->BytesPerChannel;
        output_counter += iop->BytesOfOutput;
        iop->Status = PB_MODULE_STATE_OPERATE;
        break;

      /* New style configuring (from v4.1.3) with Pb_Module objects or subclass. Loop all channels
        in the module and set channel size and offset. */	 

      case pwr_cClass_Pb_Module:
        mp = (pwr_sClass_Pb_Module *) cardp->op;
        mp->Status = PB__INITFAIL;
	cardp->offset = 0;
        for (i=0; i<cardp->ChanListSize; i++) {
          chanp = &cardp->chanlist[i];

	  if ( is_diag( &chanp->ChanAref)) {
	    chanp->udata |= PB_UDATA_DIAG;
	    switch (chanp->ChanClass) {	    
            case pwr_cClass_ChanIi:
	      chanp->offset = ((pwr_sClass_ChanIi *)chanp->cop)->Number;
	      chanp->size = GetChanSize( ((pwr_sClass_ChanIi *)chanp->cop)->Representation);
	      break;
	    default:
	      errh_Error( "Diagnostic channel class, card %s", cardp->Name);
	    }
	    continue;
	  }

          if (chanp->ChanClass != pwr_cClass_ChanDi) {
            input_counter += latent_input_count;
	    latent_input_count = 0;
          }

          if (chanp->ChanClass != pwr_cClass_ChanDo) {
            output_counter += latent_output_count;
	    latent_output_count = 0;
          }
      
          switch (chanp->ChanClass) {
      
            case pwr_cClass_ChanDi:
	      chan_di = (pwr_sClass_ChanDi *) chanp->cop;
              if (chan_di->Number == 0) {
	        input_counter += latent_input_count;
	        latent_input_count = 0;
	      }
              chanp->offset = input_counter;
	      chanp->mask = 1<<chan_di->Number;
	      if (chan_di->Representation == pwr_eDataRepEnum_Bit16 && op->ByteOrdering == pwr_eByteOrderingEnum_BigEndian) 
	        chanp->mask = swap16(chanp->mask);
	      if (chan_di->Representation == pwr_eDataRepEnum_Bit32 && op->ByteOrdering == pwr_eByteOrderingEnum_BigEndian)
	        chanp->mask = swap32((unsigned short) chanp->mask);
	      if (chan_di->Number == 0) latent_input_count = GetChanSize(chan_di->Representation);
//	      printf("Di channel found in %s, Number %d, Offset %d\n", cardp->Name, chan_di->Number, chanp->offset);
	      break;
	  
            case pwr_cClass_ChanAi:
	      chan_ai = (pwr_sClass_ChanAi *) chanp->cop;
              chanp->offset = input_counter;
	      chan_size = GetChanSize(chan_ai->Representation);
              chanp->size = chan_size;
	      chanp->mask = 0;
	      input_counter += chan_size;
              io_AiRangeToCoef(chanp);
//	      printf("Ai channel found in %s, Number %d, Offset %d\n", cardp->Name, chan_ai->Number, chanp->offset);
	      break;
	  
            case pwr_cClass_ChanAit:
	      chan_ait = (pwr_sClass_ChanAit *) chanp->cop;
              chanp->offset = input_counter;
	      chan_size = GetChanSize(chan_ait->Representation);
              chanp->size = chan_size;
	      chanp->mask = 0;
	      input_counter += chan_size;
              io_AiRangeToCoef(chanp);
	      break;
	  
            case pwr_cClass_ChanIi:
	      chan_ii = (pwr_sClass_ChanIi *) chanp->cop;
              chanp->offset = input_counter;
	      chan_size = GetChanSize(chan_ii->Representation);
              chanp->size = chan_size;
	      chanp->mask = 0;
	      input_counter += chan_size;
//	      printf("Ii channel found in %s, Number %d, Offset %d\n", cardp->Name, chan_ii->Number, chanp->offset);
	      break;
	  
            case pwr_cClass_ChanDo:
	      chan_do = (pwr_sClass_ChanDo *) chanp->cop;
              if (chan_do->Number == 0) {
	        output_counter += latent_output_count;
	        latent_output_count = 0;
	      }
              chanp->offset = output_counter;
	      chan_size = GetChanSize(chan_do->Representation);
	      chanp->mask = 1<<chan_do->Number;
	      if (chan_do->Representation == pwr_eDataRepEnum_Bit16 && op->ByteOrdering == pwr_eByteOrderingEnum_BigEndian) 
	        chanp->mask = swap16(chanp->mask);
	      if (chan_do->Representation == pwr_eDataRepEnum_Bit32 && op->ByteOrdering == pwr_eByteOrderingEnum_BigEndian)
	        chanp->mask = swap32((unsigned short) chanp->mask);
	      if (chan_do->Number == 0) latent_output_count = GetChanSize(chan_do->Representation);
//	      printf("Do channel found in %s, Number %d, Offset %d\n", cardp->Name, chan_do->Number, chanp->offset);
	      break;
	  
	    case pwr_cClass_ChanAo:
	      chan_ao = (pwr_sClass_ChanAo *) chanp->cop;
              chanp->offset = output_counter;
	      chan_size = GetChanSize(chan_ao->Representation);
              chanp->size = chan_size;
	      chanp->mask = 0;
	      output_counter += chan_size;
              io_AoRangeToCoef(chanp);
//	      printf("Ao channel found in %s, Number %d, Offset %d\n", cardp->Name, chan_ao->Number, chanp->offset);
	      break;
	  
            case pwr_cClass_ChanIo:
	      chan_io = (pwr_sClass_ChanIo *) chanp->cop;
              chanp->offset = output_counter;
	      chan_size = GetChanSize(chan_io->Representation);
              chanp->size = chan_size;
	      chanp->mask = 0;
	      output_counter += chan_size;
//	      printf("Io channel found in %s, Number %d, Offset %d\n", cardp->Name, chan_io->Number, chanp->offset);
	      break;
          }
        }

        mp->Status = PB__NOCONN;
        break;
    }

    op->NumberModules++;
    cardp = cardp->next;
  }

  return IO__SUCCESS;
}


/*----------------------------------------------------------------------------*\
   Read method for the Pb DP slave 
\*----------------------------------------------------------------------------*/
static pwr_tStatus IoRackRead (
  io_tCtx	ctx,
  io_sAgent	*ap,
  io_sRack	*rp
) 
{
  pwr_sClass_Pb_Profiboard *mp;
  pwr_sClass_Pb_DP_Slave *sp;
  
  sp = (pwr_sClass_Pb_DP_Slave *) rp->op;
  mp = (pwr_sClass_Pb_Profiboard *) ap->op;

  /* The reading of the process image is now performed at the agent level,
  this eliminates the need for board specific code at the rack level.  */

  if (sp->DisableSlave != 1 && mp->DisableBus != 1) {
  
    if (sp->Status == PB__NORMAL) {
      sp->ErrorCount = 0;
    }
    else {
      sp->ErrorCount++;
    }

    if (sp->ErrorCount > sp->ErrorSoftLimit && sp->StallAction >= pwr_ePbStallAction_ResetInputs) {
      memset(&sp->Inputs, 0, sp->BytesOfInput);
    }

    if (sp->ErrorCount > sp->ErrorHardLimit && sp->StallAction >= pwr_ePbStallAction_EmergencyBreak) {
      ctx->Node->EmergBreakTrue = 1;
    }
  }
  else {
    sp->ErrorCount = 0;
    sp->Status = PB__DISABLED;
  }
  
  return IO__SUCCESS;
}


/*----------------------------------------------------------------------------*\
   Write method for the Pb DP slave
\*----------------------------------------------------------------------------*/
static pwr_tStatus IoRackWrite (
  io_tCtx	ctx,
  io_sAgent	*ap,
  io_sRack	*rp
) 
{
  pwr_sClass_Pb_Profiboard *mp;
  pwr_sClass_Pb_DP_Slave *sp;
  
  sp = (pwr_sClass_Pb_DP_Slave *) rp->op;
  mp = (pwr_sClass_Pb_Profiboard *) ap->op;

  /* The writing of the process image is now performed at the agent level,
  this eliminates the need for board specific code at the rack level.  */

  if (sp->DisableSlave == 1 || mp->DisableBus == 1) sp->Status = PB__DISABLED;

  return IO__SUCCESS;
}


/*----------------------------------------------------------------------------*\
  
\*----------------------------------------------------------------------------*/
static pwr_tStatus IoRackClose (
  io_tCtx	ctx,
  io_sAgent	*ap,
  io_sRack	*rp
) 
{
  return IO__SUCCESS;
}


/*----------------------------------------------------------------------------*\
  Every method to be exported to the workbench should be registred here.
\*----------------------------------------------------------------------------*/

pwr_dExport pwr_BindIoMethods(Pb_DP_Slave) = {
  pwr_BindIoMethod(IoRackInit),
  pwr_BindIoMethod(IoRackRead),
  pwr_BindIoMethod(IoRackWrite),
  pwr_BindIoMethod(IoRackClose),
  pwr_NullMethod
};