/* 
 * Proview   $Id$
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
 **/

/* rt_pn_iface.cpp -- Profinet driver interface routines  */

#include <vector>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pwr.h"
#include "co_cdh.h"
#include "pwr_baseclasses.h"
#include "pwr_profibusclasses.h"
#include "rt_io_base.h"
#include "rt_io_bus.h"
#include "rt_io_msg.h"
#include "rt_errh.h"
#include "rt_pb_msg.h"

#include "rt_profinet.h"
#include "rt_pnak.h"
#include "co_dcli.h"
#include "rt_pn_gsdml_data.h"
#include "rt_io_base.h"
#include "rt_io_pn_locals.h"

#define _PN_U32_HIGH_WORD(U32)                                     ((PN_U16) ((U32) >> 16))
#define _PN_U32_LOW_WORD(U32)                                      ((PN_U16)  (U32)       )

#define _PN_U32_HIGH_HIGH_BYTE(U32)                                     ((PN_U8) ((U32) >> 24))
#define _PN_U32_HIGH_LOW_BYTE(U32)                                     ((PN_U8) ((U32) >> 16))
#define _PN_U32_LOW_HIGH_BYTE(U32)                                     ((PN_U8) ((U32) >> 8))
#define _PN_U32_LOW_LOW_BYTE(U32)                                     ((PN_U8) (U32) )


#define _PN_U16_HIGH_BYTE(U16)                                     ((PN_U8) ((U16) >> 8))
#define _PN_U16_LOW_BYTE(U16)                                      ((PN_U8)  (U16)      )

#define _HIGH_LOW_WORDS_TO_PN_U32(High,Low)                        ((((PN_U32) (High)) << 16) + (Low))
#define _HIGH_LOW_BYTES_TO_PN_U16(High,Low)                        ((PN_U16) ((((PN_U16) (High)) << 8 ) + (Low)))
#define _HIGH_LOW_BYTES_TO_PN_U32(hwhb, hwlb, lwhb, lwlb)                        ((PN_U32) ((((PN_U32) (hwhb)) << 24 ) + (((PN_U32) (hwlb)) << 16 ) + (((PN_U32) (lwhb)) << 8 ) + (lwlb)))

#define _
#define NUM_DEVICES 2

char   file_vect[2][80] = {
  "pwr_pn_000_001_099_020_000000a2.xml",
  "pwr_pn_000_001_099_020_000000e5.xml",
    };

void pack_set_ip_settings_req(T_PNAK_SERVICE_REQ_RES *ServiceReqRes, PnDeviceInfo *dev_info)
{
  T_PNAK_SERVICE_DESCRIPTION  *service_desc;
  T_PN_SERVICE_SET_IP_SETTINGS_REQ  *pSISR;
  unsigned offset = 0u;

  memset(ServiceReqRes, 0, sizeof(T_PNAK_SERVICE_REQ_RES));
  ServiceReqRes->NumberEntries = 1;
  ServiceReqRes->ServiceEntry[0].ServiceOffset = 0;

  service_desc = (T_PNAK_SERVICE_DESCRIPTION *) &ServiceReqRes->ServiceChannel[offset];

  service_desc->DeviceRef  = 0u;
  service_desc->Instance   = PN_SUPERVISOR;
  service_desc->Service    = PN_SERVICE_SET_IP_SETTINGS;
  service_desc->Primitive  = PNAK_SERVICE_REQ;
  service_desc->ClientId   = 0;
  service_desc->InvokeId   = 0;
  service_desc->DataLength = sizeof(T_PN_SERVICE_SET_IP_SETTINGS_REQ);

  /* Fill service */

  pSISR = (T_PN_SERVICE_SET_IP_SETTINGS_REQ *) (service_desc + 1);

  //  no_items = sscanf(dev_data->mac_address, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &havhb, &havlb, &mhb, &mlb, &lhb, &llb);

  pSISR->DestMacAddress.HighAndVersionHighByte = dev_info->macaddress[5];
  pSISR->DestMacAddress.HighAndVersionLowByte  = dev_info->macaddress[4];
  pSISR->DestMacAddress.MidHighByte            = dev_info->macaddress[3];
  pSISR->DestMacAddress.MidLowByte             = dev_info->macaddress[2];
  pSISR->DestMacAddress.LowHighByte            = dev_info->macaddress[1];
  pSISR->DestMacAddress.LowLowByte             = dev_info->macaddress[0];

  //  no_items = sscanf(dev_data->ip_address, "%hhi.%hhi.%hhi.%hhi", &high_high_byte,
  //		    &high_low_byte, &low_high_byte, &low_low_byte);

  //  if (no_items == 4) {
  pSISR->AddressHighWordHighByte = dev_info->ipaddress[3];
  pSISR->AddressHighWordLowByte  = dev_info->ipaddress[2];
  pSISR->AddressLowWordHighByte  = dev_info->ipaddress[1];
  pSISR->AddressLowWordLowByte   = dev_info->ipaddress[0];

  //  no_items = sscanf(dev_data->subnet_mask, "%hhi.%hhi.%hhi.%hhi", &high_high_byte,
  //		    &high_low_byte, &low_high_byte, &low_low_byte);

//  if (no_items == 4) {
    pSISR->SubnetMaskHighWordHighByte = dev_info->subnetmask[3];
    pSISR->SubnetMaskHighWordLowByte  = dev_info->subnetmask[2];
    pSISR->SubnetMaskLowWordHighByte  = dev_info->subnetmask[1];
    pSISR->SubnetMaskLowWordLowByte   = dev_info->subnetmask[0];
}
void pack_set_device_name_req(T_PNAK_SERVICE_REQ_RES *ServiceReqRes, PnDeviceInfo *dev_info)
{
  T_PNAK_SERVICE_DESCRIPTION  *service_desc;
  T_PN_SERVICE_SET_NAME_REQ  *pSNR;
  unsigned short name_length;
  unsigned offset = 0u;

  memset(ServiceReqRes, 0, sizeof(T_PNAK_SERVICE_REQ_RES));
  ServiceReqRes->NumberEntries = 1;
  ServiceReqRes->ServiceEntry[0].ServiceOffset = 0;

  service_desc = (T_PNAK_SERVICE_DESCRIPTION *) &ServiceReqRes->ServiceChannel[offset];

  service_desc->DeviceRef  = 0u;
  service_desc->Instance   = PN_SUPERVISOR;
  service_desc->Service    = PN_SERVICE_SET_DEVICE_NAME;
  service_desc->Primitive  = PNAK_SERVICE_REQ;
  service_desc->ClientId   = 0;
  service_desc->InvokeId   = 0;
  service_desc->DataLength = sizeof(T_PN_SERVICE_SET_NAME_REQ);

  /* Fill service */

  pSNR = (T_PN_SERVICE_SET_NAME_REQ *) (service_desc + 1);

  //  no_items = sscanf(dev_data->mac_address, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &havhb, &havlb, &mhb, &mlb, &lhb, &llb);

  pSNR->DestMacAddress.HighAndVersionHighByte = dev_info->macaddress[5];
  pSNR->DestMacAddress.HighAndVersionLowByte  = dev_info->macaddress[4];
  pSNR->DestMacAddress.MidHighByte            = dev_info->macaddress[3];
  pSNR->DestMacAddress.MidLowByte             = dev_info->macaddress[2];
  pSNR->DestMacAddress.LowHighByte            = dev_info->macaddress[1];
  pSNR->DestMacAddress.LowLowByte             = dev_info->macaddress[0];

  name_length = strlen(dev_info->devname);

  pSNR->NameLengthHighByte =  _PN_U16_HIGH_BYTE(name_length);
  pSNR->NameLengthLowByte  =  _PN_U16_LOW_BYTE(name_length);

  strncpy(pSNR->DeviceName, dev_info->devname, PN_MAX_DEVICE_NAME_LENGTH);
}
void pack_set_identification_req(T_PNAK_SERVICE_REQ_RES *ServiceReqRes)
{
  unsigned offset = 0u;

  T_PNAK_SERVICE_DESCRIPTION  *service_desc;
  T_PN_SERVICE_SET_IDENTIFICATION_REQ  *pSSIR;

  memset(ServiceReqRes, 0, sizeof(T_PNAK_SERVICE_REQ_RES));
  ServiceReqRes->NumberEntries = 1;
  ServiceReqRes->ServiceEntry[0].ServiceOffset = 0;
 
  service_desc = (T_PNAK_SERVICE_DESCRIPTION *) &ServiceReqRes->ServiceChannel[offset];

  service_desc->DeviceRef  = PN_DEVICE_REFERENCE_THIS_STATION;
  service_desc->Instance   = PN;
  service_desc->Service    = PN_SERVICE_SET_IDENTIFICATION;
  service_desc->Primitive  = PNAK_SERVICE_REQ;
  service_desc->ClientId   = 0;
  service_desc->InvokeId   = 0;
 
  /* Calculate length of service */

  service_desc->DataLength = sizeof(T_PN_SERVICE_SET_IDENTIFICATION_REQ);

  /* Fill service */

  pSSIR = (T_PN_SERVICE_SET_IDENTIFICATION_REQ *) (service_desc + 1);

  sprintf((char *) pSSIR->TypeOfStation, "PN-IO-CON-OC-Implementation");
  sprintf((char *) pSSIR->OrderId, "PN-IO-CON-OC       ");
  sprintf((char *) pSSIR->SerialNumber, "0700123456789099");
  pSSIR->HwRevisionHighByte = 0;
  pSSIR->HwRevisionLowByte = 1;
  pSSIR->SwRevisionHighWordHighByte = 0;
  pSSIR->SwRevisionHighWordLowByte = 60;
  pSSIR->SwRevisionLowWordHighByte = 0;
  pSSIR->SwRevisionLowWordLowByte = 'V'; 
  pSSIR->IdentAndMaintenanceVersionHighByte = 1;
  pSSIR->IdentAndMaintenanceVersionLowByte = 1;
}

void pack_get_device_state_req(T_PNAK_SERVICE_REQ_RES *ServiceReqRes, unsigned short device_ref)
{
  unsigned offset = 0u;

  T_PNAK_SERVICE_DESCRIPTION  *service_desc;

  memset(ServiceReqRes, 0, sizeof(T_PNAK_SERVICE_REQ_RES));
  ServiceReqRes->NumberEntries = 1;
  ServiceReqRes->ServiceEntry[0].ServiceOffset = 0;
 
  service_desc = (T_PNAK_SERVICE_DESCRIPTION *) &ServiceReqRes->ServiceChannel[offset];

  service_desc->DeviceRef  = device_ref;
  service_desc->Instance   = PN_CONTROLLER;
  service_desc->Service    = PN_SERVICE_GET_DEVICE_STATE;
  service_desc->Primitive  = PNAK_SERVICE_REQ;
  service_desc->ClientId   = 0;
  service_desc->InvokeId   = 0;
 
  /* Calculate length of service */

  service_desc->DataLength = sizeof(T_PN_SERVICE_GET_DEVICE_STATE_REQ);

}

void pack_get_los_req(T_PNAK_SERVICE_REQ_RES *ServiceReqRes)
{
  unsigned offset = 0u;

  T_PNAK_SERVICE_DESCRIPTION  *service_desc;

  memset(ServiceReqRes, 0, sizeof(T_PNAK_SERVICE_REQ_RES));
  ServiceReqRes->NumberEntries = 1;
  ServiceReqRes->ServiceEntry[0].ServiceOffset = 0;
 
  service_desc = (T_PNAK_SERVICE_DESCRIPTION *) &ServiceReqRes->ServiceChannel[offset];

  service_desc->DeviceRef  = PN_DEVICE_REFERENCE_THIS_STATION;
  service_desc->Instance   = PN_SUPERVISOR;
  service_desc->Service    = PN_SERVICE_GET_LIST_OF_STATION;
  service_desc->Primitive  = PNAK_SERVICE_REQ;
  service_desc->ClientId   = 0;
  service_desc->InvokeId   = 0;
 
  /* Calculate length of service */

  service_desc->DataLength = sizeof(T_PN_SERVICE_GET_LIST_OF_STATION_REQ);

}

void pack_get_alarm_req(T_PNAK_SERVICE_REQ_RES *ServiceReqRes, unsigned short ref,  unsigned short device_ref)
{
  unsigned offset = 0u;

  T_PNAK_SERVICE_DESCRIPTION  *service_desc;
  T_PN_SERVICE_GET_ALARM_REQ *pGAR;

  memset(ServiceReqRes, 0, sizeof(T_PNAK_SERVICE_REQ_RES));
  ServiceReqRes->NumberEntries = 1;
  ServiceReqRes->ServiceEntry[0].ServiceOffset = 0;
 
  service_desc = (T_PNAK_SERVICE_DESCRIPTION *) &ServiceReqRes->ServiceChannel[offset];

  service_desc->DeviceRef  = device_ref;
  service_desc->Instance   = PN;
  service_desc->Service    = PN_SERVICE_GET_ALARM;
  service_desc->Primitive  = PNAK_SERVICE_REQ;
  service_desc->ClientId   = 0;
  service_desc->InvokeId   = 0;
 
  /* Calculate length of service */

  service_desc->DataLength = sizeof(T_PN_SERVICE_GET_ALARM_REQ);

  pGAR = (T_PN_SERVICE_GET_ALARM_REQ *) (service_desc + 1);

  pGAR->AlarmRefHighByte = _PN_U16_HIGH_BYTE(ref);
  pGAR->AlarmRefLowByte = _PN_U16_LOW_BYTE(ref);

}

void pack_alarm_ack_req(T_PNAK_SERVICE_REQ_RES *ServiceReqRes, unsigned short ref, unsigned short prio, unsigned short device_ref)
{
  unsigned offset = 0u;

  T_PNAK_SERVICE_DESCRIPTION  *service_desc;
  T_PN_SERVICE_ALARM_ACK_REQ *pAAR;

  memset(ServiceReqRes, 0, sizeof(T_PNAK_SERVICE_REQ_RES));
  ServiceReqRes->NumberEntries = 1;
  ServiceReqRes->ServiceEntry[0].ServiceOffset = 0;
 
  service_desc = (T_PNAK_SERVICE_DESCRIPTION *) &ServiceReqRes->ServiceChannel[offset];

  service_desc->DeviceRef  = device_ref;
  service_desc->Instance   = PN;
  service_desc->Service    = PN_SERVICE_ALARM_ACK;
  service_desc->Primitive  = PNAK_SERVICE_REQ;
  service_desc->ClientId   = 0;
  service_desc->InvokeId   = 0;
 
  /* Calculate length of service */

  service_desc->DataLength = sizeof(T_PN_SERVICE_ALARM_ACK_REQ);

  pAAR = (T_PN_SERVICE_ALARM_ACK_REQ *) (service_desc + 1);

  pAAR->AlarmRefHighByte = _PN_U16_HIGH_BYTE(ref);
  pAAR->AlarmRefLowByte  = _PN_U16_LOW_BYTE(ref);
  pAAR->AlarmPriority    = _PN_U16_LOW_BYTE(prio);
}

void pack_download_req(T_PNAK_SERVICE_REQ_RES *ServiceReqRes, GsdmlDeviceData *dev_data, unsigned short device_ref)
{
  unsigned offset = 0u;

  T_PNAK_SERVICE_DESCRIPTION  *service_desc;

  unsigned short num_iocrs = 0;
  unsigned short num_apis = 0;
  unsigned short num_modules = 0;
  unsigned short num_submodules = 0;
  unsigned short num_datarecords = 0;
  unsigned short num_sm;
  char           *pData;
  unsigned short module_ind = 0;
  unsigned short datarecord_ind = 0;
  unsigned short data_record_length = 0;
  unsigned short no_items;
  unsigned short ii, jj, kk, length;
  static unsigned short phase = 1;
  //  static unsigned short old_red_ratio = 1;
  


  memset(ServiceReqRes, 0, sizeof(T_PNAK_SERVICE_REQ_RES));
  ServiceReqRes->NumberEntries = 1;
  ServiceReqRes->ServiceEntry[0].ServiceOffset = 0;
 
  service_desc = (T_PNAK_SERVICE_DESCRIPTION *) &ServiceReqRes->ServiceChannel[offset];

  service_desc->DeviceRef  = device_ref;
  //  service_desc->DeviceRef  = PN_DEVICE_REFERENCE_THIS_STATION;
  service_desc->Instance   = PN; // PN_CONTROLLER
  service_desc->Service    = PN_SERVICE_DOWNLOAD;
  service_desc->Primitive  = PNAK_SERVICE_REQ;
  service_desc->ClientId   = 0;
  service_desc->InvokeId   = 0;
 
  /* Calculate length of service */

  num_iocrs = dev_data->iocr_data.size();

  if (num_iocrs > 0)
    num_apis = 1;

  for (ii = 0; ii < dev_data->slot_data.size(); ii++) {
    if ((dev_data->slot_data[ii]->module_enum_number != 0) ||
	(ii == 0))
      num_modules++;
    else break;
  }

  //  num_modules = dev_data->slot_data.size();

  for (ii = 0; ii < num_modules; ii++) {
    num_sm = dev_data->slot_data[ii]->subslot_data.size();
    num_submodules += num_sm;
    for (jj = 0; jj < num_sm; jj++ ) {
      num_datarecords += dev_data->slot_data[ii]->subslot_data[jj]->data_record.size();
      for (kk = 0; kk < dev_data->slot_data[ii]->subslot_data[jj]->data_record.size(); kk++) {
        data_record_length += dev_data->slot_data[ii]->subslot_data[jj]->data_record[kk]->data_length;
      }
    }
  }

  length = sizeof(T_PN_SERVICE_DOWNLOAD_REQ) + 
    num_iocrs * sizeof(T_PN_IOCR) + num_apis * sizeof(T_PN_API) +
    num_modules * sizeof(T_PN_MODULE) + num_submodules * sizeof(T_PN_SUBMODULE) +
    num_datarecords * sizeof(T_PN_DATA_RECORD) + 
    (num_iocrs * num_apis + num_modules + num_datarecords) * sizeof(T_PN_REFERENCE) +
    data_record_length;

  service_desc->DataLength = length;

  pData = (char *) (service_desc + 1);

  /* Fill data in download request */

  T_PN_SERVICE_DOWNLOAD_REQ *pSDR = (T_PN_SERVICE_DOWNLOAD_REQ *) pData;

  unsigned char   high_high_byte;
  unsigned char   high_low_byte;
  unsigned char   low_high_byte;
  unsigned char   low_low_byte;
  unsigned char   high_byte;
  unsigned char   low_byte;
  unsigned long   ar_property;
  unsigned short  high_word;
  unsigned short  low_word;
  unsigned short  sub_prop;
  T_PN_IOCR      *pIOCR;
  T_PN_REFERENCE *pAPIReference;
  T_PN_REFERENCE *pModuleReference;
  T_PN_MODULE    *pModule;
  T_PN_SUBMODULE *pSubModule;
  T_PN_REFERENCE *pDataRecordReference;
  T_PN_DATA_RECORD *pDataRecord;

  if (device_ref == 1) {
    //    printf("sizeof download-struct: %d\n", sizeof(T_PN_SERVICE_DOWNLOAD_REQ));
    //    printf("sizeof IOCR-struct: %d\n", sizeof(T_PN_IOCR));  
    //    printf("sizeof API-struct: %d\n", sizeof(T_PN_API));
    //    printf("sizeof MODULE-struct: %d\n", sizeof(T_PN_MODULE));
    //    printf("sizeof SUBMODULE-struct: %d\n", sizeof(T_PN_SUBMODULE));
    //    printf("sizeof DATARECORD-struct: %d\n", sizeof(T_PN_DATA_RECORD));
    //    printf("sizeof REFERENCE-struct: %d\n", sizeof(T_PN_REFERENCE));
  }

  no_items = sscanf(dev_data->ip_address, "%hhi.%hhi.%hhi.%hhi", &high_high_byte,
	 &high_low_byte, &low_high_byte, &low_low_byte);

  if (no_items == 4) {
    pSDR->IpAddressHighWordHighByte = high_high_byte;
    pSDR->IpAddressHighWordLowByte  = high_low_byte;
    pSDR->IpAddressLowWordHighByte  = low_high_byte;
    pSDR->IpAddressLowWordLowByte   = low_low_byte;
  }

  no_items = sscanf(dev_data->subnet_mask, "%hhi.%hhi.%hhi.%hhi", &high_high_byte,
	 &high_low_byte, &low_high_byte, &low_low_byte);

  if (no_items == 4) {
    pSDR->SubnetMaskHighWordHighByte = high_high_byte;
    pSDR->SubnetMaskHighWordLowByte  = high_low_byte;
    pSDR->SubnetMaskLowWordHighByte  = low_high_byte;
    pSDR->SubnetMaskLowWordLowByte   = low_low_byte;
  }

  sprintf(pSDR->DeviceName, dev_data->device_name);

  if (device_ref == PN_DEVICE_REFERENCE_THIS_STATION) {
    sprintf(pSDR->InterfaceName,  dev_data->device_text);
    pSDR->Flag = PN_SERVICE_DOWNLOAD_FLAG_AUTO_CONTROL_RES;
    ar_property =  0;
    pSDR->InstanceLowByte = 0;
  }
  else {
    pSDR->Flag = PN_SERVICE_DOWNLOAD_FLAG_ACTIVATE;
    ar_property =  PROFINET_AR_PROPERTY_STATE_PRIMARY | 
      PROFINET_AR_PROPERTY_PARAMETER_SERVER_CM | 
      PROFINET_AR_PROPERTY_DATA_RATE_100MBIT;
    pSDR->InstanceHighByte = _PN_U16_HIGH_BYTE(dev_data->instance);
    pSDR->InstanceLowByte  = _PN_U16_LOW_BYTE(dev_data->instance);
  }

  no_items = sscanf(dev_data->version, "%hhi.%hhi", &high_byte, &low_byte);

  pSDR->VersionHighByte = high_byte;
  pSDR->VersionLowByte  = low_byte;

  high_word = _PN_U32_HIGH_WORD(ar_property);
  low_word  = _PN_U32_LOW_WORD(ar_property);

  pSDR->ARPropertiesHighWordHighByte = _PN_U16_HIGH_BYTE(high_word);
  pSDR->ARPropertiesHighWordLowByte  = _PN_U16_LOW_BYTE(high_word);
  pSDR->ARPropertiesLowWordHighByte  = _PN_U16_HIGH_BYTE(low_word);
  pSDR->ARPropertiesLowWordLowByte   = _PN_U16_LOW_BYTE(low_word);

  pSDR->DeviceIdHighByte = _PN_U16_HIGH_BYTE(dev_data->device_id);
  pSDR->DeviceIdLowByte  = _PN_U16_LOW_BYTE(dev_data->device_id);
  pSDR->VendorIdHighByte = _PN_U16_HIGH_BYTE(dev_data->vendor_id);
  pSDR->VendorIdLowByte  = _PN_U16_LOW_BYTE(dev_data->vendor_id);

  pSDR->NumberOfIOCRHighByte = _PN_U16_HIGH_BYTE(num_iocrs);
  pSDR->NumberOfIOCRLowByte  = _PN_U16_LOW_BYTE(num_iocrs);
  pSDR->NumberOfAPIsHighByte = _PN_U16_HIGH_BYTE(num_apis);
  pSDR->NumberOfAPIsLowByte  = _PN_U16_LOW_BYTE(num_apis);
  pSDR->NumberOfModulesHighByte = _PN_U16_HIGH_BYTE(num_modules);
  pSDR->NumberOfModulesLowByte  = _PN_U16_LOW_BYTE(num_modules);
  pSDR->NumberOfDataRecordsHighByte = _PN_U16_HIGH_BYTE(num_datarecords);
  pSDR->NumberOfDataRecordsLowByte  = _PN_U16_LOW_BYTE(num_datarecords);

  pSDR->AlarmCRBlock.VersionHighByte = 1;
  pSDR->AlarmCRBlock.VersionLowByte  = 0;
  pSDR->AlarmCRBlock.RTATimeoutFactorHighByte = 0;
  pSDR->AlarmCRBlock.RTATimeoutFactorLowByte  = 1;
  pSDR->AlarmCRBlock.RTARetryHighByte = 0;
  pSDR->AlarmCRBlock.RTARetryLowByte  = 3;
  pSDR->AlarmCRBlock.PropertiesHighWordHighByte = 0;
  pSDR->AlarmCRBlock.PropertiesHighWordLowByte  = 0;
  pSDR->AlarmCRBlock.PropertiesLowWordHighByte  = 0;
  pSDR->AlarmCRBlock.PropertiesLowWordLowByte   = 0;
  pSDR->AlarmCRBlock.MaxAlarmLengthHighByte = 0;
  pSDR->AlarmCRBlock.MaxAlarmLengthLowByte = 200;
  pSDR->AlarmCRBlock.TagHeaderHighHighByte = 0xC0;
  pSDR->AlarmCRBlock.TagHeaderHighLowByte  = 0;
  pSDR->AlarmCRBlock.TagHeaderLowHighByte  = 0xA0;
  pSDR->AlarmCRBlock.TagHeaderLowLowByte   = 0;


  /* Fill the IOCRS's */

  pIOCR = (T_PN_IOCR *) (pSDR + 1);

  for (ii = 0; ii < num_iocrs; ii++) {
    /* Fill data for IOCR */

    pIOCR->VersionHighByte = pSDR->VersionHighByte;
    pIOCR->VersionLowByte  = pSDR->VersionLowByte;
    pIOCR->TypeHighByte = _PN_U16_HIGH_BYTE(dev_data->iocr_data[ii]->type);
    pIOCR->TypeLowByte  = _PN_U16_LOW_BYTE(dev_data->iocr_data[ii]->type);
    pIOCR->PropertiesHighWordHighByte = _PN_U32_HIGH_HIGH_BYTE(dev_data->iocr_data[ii]->properties);
    pIOCR->PropertiesHighWordLowByte  = _PN_U32_HIGH_LOW_BYTE(dev_data->iocr_data[ii]->properties);
    pIOCR->PropertiesLowWordHighByte  = _PN_U32_LOW_HIGH_BYTE(dev_data->iocr_data[ii]->properties);
    pIOCR->PropertiesLowWordLowByte   = _PN_U32_LOW_LOW_BYTE(dev_data->iocr_data[ii]->properties);
    pIOCR->SendClockFactorHighByte = _PN_U16_HIGH_BYTE(dev_data->iocr_data[ii]->send_clock_factor);
    pIOCR->SendClockFactorLowByte  = _PN_U16_LOW_BYTE(dev_data->iocr_data[ii]->send_clock_factor);

    if (dev_data->iocr_data[ii]->reduction_ratio < 1) {
      dev_data->iocr_data[ii]->reduction_ratio = 1;
    }
    pIOCR->ReductionRatioHighByte = _PN_U16_HIGH_BYTE(dev_data->iocr_data[ii]->reduction_ratio);
    pIOCR->ReductionRatioLowByte  = _PN_U16_LOW_BYTE(dev_data->iocr_data[ii]->reduction_ratio);

    if (dev_data->iocr_data[ii]->reduction_ratio == 0)
      dev_data->iocr_data[ii]->reduction_ratio = 1;

    /*    if (old_red_ratio == dev_data->iocr_data[ii]->reduction_ratio) {
      phase = (phase + 1) % old_red_ratio + 1;
    } else {
      phase = 1;
      old_red_ratio = dev_data->iocr_data[ii]->reduction_ratio;
      }*/
    phase = 1;
    //    pIOCR->ReductionRatioHighByte = 0;
    //    pIOCR->ReductionRatioLowByte  = 128;
    pIOCR->PhaseHighByte = _PN_U16_HIGH_BYTE(phase);
    pIOCR->PhaseLowByte  = _PN_U16_LOW_BYTE(phase);;
    pIOCR->SequenceHighByte = 0;
    pIOCR->SequenceLowByte  = 0;
    pIOCR->WatchdogFactorHighByte = 0;
    pIOCR->WatchdogFactorLowByte  = 3;
    pIOCR->DataHoldFactorHighByte = 0;
    pIOCR->DataHoldFactorLowByte  = 3;
    pIOCR->FrameSendOffsetHighWordHighByte = 0xFF; // As fast as possible
    pIOCR->FrameSendOffsetHighWordLowByte  = 0xFF;
    pIOCR->FrameSendOffsetLowWordHighByte  = 0xFF;
    pIOCR->FrameSendOffsetLowWordLowByte   = 0xFF;
    pIOCR->TagHeaderHighByte = 0xC0;
    pIOCR->TagHeaderLowByte  = 0;
    pIOCR->MulticastAddr.HighAndVersionHighByte = 0;
    pIOCR->MulticastAddr.HighAndVersionLowByte  = 0;
    pIOCR->MulticastAddr.MidHighByte            = 0;
    pIOCR->MulticastAddr.MidLowByte             = 0;
    pIOCR->MulticastAddr.LowHighByte            = 0;
    pIOCR->MulticastAddr.LowLowByte             = 0;
    //    pIOCR->NumberOfAPIsHighByte = _PN_U16_HIGH_BYTE(num_apis);
    //    pIOCR->NumberOfAPIsLowByte  = _PN_U16_HIGH_BYTE(num_apis);
    pIOCR->NumberOfAPIsHighByte = _PN_U16_HIGH_BYTE(num_apis);
    pIOCR->NumberOfAPIsLowByte  = _PN_U16_LOW_BYTE(num_apis);

    /* Fill references to API */

    pAPIReference = (T_PN_REFERENCE *) (pIOCR + 1);

    for (jj = 0; jj < num_apis; jj++) {
      pAPIReference->ReferenceHighByte = _PN_U16_HIGH_BYTE(jj); 
      pAPIReference->ReferenceLowByte  = _PN_U16_LOW_BYTE(jj); 
      pAPIReference++;
    }

    pIOCR = (T_PN_IOCR *) pAPIReference;
  }

  /* Fill the API's */

  T_PN_API *pAPI = (T_PN_API *) pIOCR;

  for (ii = 0; ii < num_apis; ii++) {

    /* Fill data for API */
  
    pAPI->APIHighWordHighByte = _PN_U32_HIGH_HIGH_BYTE(PROFINET_DEFAULT_API);
    pAPI->APIHighWordLowByte  = _PN_U32_HIGH_LOW_BYTE(PROFINET_DEFAULT_API);
    pAPI->APILowWordHighByte  = _PN_U32_LOW_HIGH_BYTE(PROFINET_DEFAULT_API);
    pAPI->APILowWordLowByte   = _PN_U32_LOW_LOW_BYTE(PROFINET_DEFAULT_API);

    pAPI->NumberOfModulesHighByte = _PN_U16_HIGH_BYTE(num_modules);
    pAPI->NumberOfModulesLowByte  = _PN_U16_LOW_BYTE(num_modules);

    /* Fill references to Modules */

    pModuleReference = (T_PN_REFERENCE *) (pAPI + 1);

    for (module_ind = 0; module_ind < num_modules; module_ind++) {
      pModuleReference->ReferenceHighByte = _PN_U16_HIGH_BYTE(module_ind); 
      pModuleReference->ReferenceLowByte  = _PN_U16_LOW_BYTE(module_ind); 
      pModuleReference++;
    }

    pAPI = (T_PN_API *) pModuleReference;
  }

  /* Fill the MODULE's */

  pModule = (T_PN_MODULE *) pAPI;

  for (ii = 0; ii < num_modules; ii++) {

    /* Fill data for MODULE */

    pModule->VersionHighByte = pSDR->VersionHighByte; 
    pModule->VersionLowByte  = pSDR->VersionLowByte; 
    pModule->SlotNumberHighByte = _PN_U16_HIGH_BYTE(dev_data->slot_data[ii]->slot_number);
    pModule->SlotNumberLowByte = _PN_U16_LOW_BYTE(dev_data->slot_data[ii]->slot_number);
    pModule->IdentNumberHighWordHighByte = _PN_U32_HIGH_HIGH_BYTE(dev_data->slot_data[ii]->module_ident_number);
    pModule->IdentNumberHighWordLowByte  = _PN_U32_HIGH_LOW_BYTE(dev_data->slot_data[ii]->module_ident_number);
    pModule->IdentNumberLowWordHighByte  = _PN_U32_LOW_HIGH_BYTE(dev_data->slot_data[ii]->module_ident_number);
    pModule->IdentNumberLowWordLowByte  = _PN_U32_LOW_LOW_BYTE(dev_data->slot_data[ii]->module_ident_number);
    pModule->PropertiesHighByte = 0; 
    pModule->PropertiesLowByte  = 0; 
    pModule->NumberOfSubmodulesHighByte = _PN_U16_HIGH_BYTE(dev_data->slot_data[ii]->subslot_data.size()); 
    pModule->NumberOfSubmodulesLowByte  = _PN_U16_LOW_BYTE(dev_data->slot_data[ii]->subslot_data.size());  

    /* Fill the SUBMODULE's */

    pSubModule = (T_PN_SUBMODULE *) (pModule + 1);

    for (jj = 0; jj < dev_data->slot_data[ii]->subslot_data.size(); jj++) {

      /* Fill data for the submodule */

      pSubModule->SubSlotNumberHighByte = _PN_U16_HIGH_BYTE(dev_data->slot_data[ii]->subslot_data[jj]->subslot_number);
      pSubModule->SubSlotNumberLowByte  = _PN_U16_LOW_BYTE(dev_data->slot_data[ii]->subslot_data[jj]->subslot_number);
      pSubModule->IdentNumberHighWordHighByte = _PN_U32_HIGH_HIGH_BYTE(dev_data->slot_data[ii]->subslot_data[jj]->submodule_ident_number);
      pSubModule->IdentNumberHighWordLowByte  = _PN_U32_HIGH_LOW_BYTE(dev_data->slot_data[ii]->subslot_data[jj]->submodule_ident_number);
      pSubModule->IdentNumberLowWordHighByte  = _PN_U32_LOW_HIGH_BYTE(dev_data->slot_data[ii]->subslot_data[jj]->submodule_ident_number);
      pSubModule->IdentNumberLowWordLowByte  = _PN_U32_LOW_LOW_BYTE(dev_data->slot_data[ii]->subslot_data[jj]->submodule_ident_number);

      if ((dev_data->slot_data[ii]->subslot_data[jj]->io_input_length > 0) &&
	  (dev_data->slot_data[ii]->subslot_data[jj]->io_output_length)) {
	sub_prop = PROFINET_IO_SUBMODULE_TYPE_INPUT_AND_OUTPUT;
      } else if (dev_data->slot_data[ii]->subslot_data[jj]->io_input_length > 0) {
	sub_prop = PROFINET_IO_SUBMODULE_TYPE_INPUT;
      } else if (dev_data->slot_data[ii]->subslot_data[jj]->io_output_length) {
	sub_prop = PROFINET_IO_SUBMODULE_TYPE_OUTPUT;
      } else {
	sub_prop = PROFINET_IO_SUBMODULE_TYPE_NO_INPUT_NO_OUTPUT;
      }

      pSubModule->PropertiesHighByte = _PN_U16_HIGH_BYTE(sub_prop); 
      pSubModule->PropertiesLowByte  = _PN_U16_LOW_BYTE(sub_prop); 
      pSubModule->InputDataLengthHighByte = _PN_U16_HIGH_BYTE(dev_data->slot_data[ii]->subslot_data[jj]->io_input_length); 
      pSubModule->InputDataLengthLowByte  = _PN_U16_LOW_BYTE(dev_data->slot_data[ii]->subslot_data[jj]->io_input_length); 
      pSubModule->OutputDataLengthHighByte = _PN_U16_HIGH_BYTE(dev_data->slot_data[ii]->subslot_data[jj]->io_output_length); 
      pSubModule->OutputDataLengthLowByte  = _PN_U16_LOW_BYTE(dev_data->slot_data[ii]->subslot_data[jj]->io_output_length); 
      pSubModule->ConsumerStatusLength = 1; // Fixed by Profinet-spec 
      pSubModule->ProviderStatusLength = 1; 
      pSubModule->NumberOfDataRecordsHighByte = _PN_U16_HIGH_BYTE(dev_data->slot_data[ii]->subslot_data[jj]->data_record.size());
      pSubModule->NumberOfDataRecordsLowByte  = _PN_U16_LOW_BYTE(dev_data->slot_data[ii]->subslot_data[jj]->data_record.size()); 

      /* Add number of datarecords */

      pDataRecordReference = (T_PN_REFERENCE *) (pSubModule + 1);

      for (kk = 0; kk < dev_data->slot_data[ii]->subslot_data[jj]->data_record.size(); kk++) {
	pDataRecordReference->ReferenceHighByte = _PN_U16_HIGH_BYTE(datarecord_ind); 
	pDataRecordReference->ReferenceLowByte  = _PN_U16_LOW_BYTE(datarecord_ind); 
	pDataRecordReference++;
        datarecord_ind++;
      }
      
      pSubModule = (T_PN_SUBMODULE *) pDataRecordReference;
    }

    pModule = (T_PN_MODULE *) pSubModule;
  }

  /* Fill the DATA_RECORD's */

  pDataRecord = (T_PN_DATA_RECORD *) pModule;

  for (ii = 0; ii < num_modules; ii++) {

    for (jj = 0; jj < dev_data->slot_data[ii]->subslot_data.size(); jj++) {
      for (kk = 0; kk < dev_data->slot_data[ii]->subslot_data[jj]->data_record.size(); kk++) {
	pDataRecord->VersionHighByte = pSDR->VersionHighByte; 
	pDataRecord->VersionLowByte  = pSDR->VersionLowByte; 
	pDataRecord->SequenceHighByte = _PN_U16_HIGH_BYTE(dev_data->slot_data[ii]->subslot_data[jj]->data_record[kk]->transfer_sequence);
	pDataRecord->SequenceLowByte  = _PN_U16_LOW_BYTE(dev_data->slot_data[ii]->subslot_data[jj]->data_record[kk]->transfer_sequence);
	pDataRecord->APIHighWordHighByte = _PN_U32_HIGH_HIGH_BYTE(PROFINET_DEFAULT_API);
	pDataRecord->APIHighWordLowByte  = _PN_U32_HIGH_LOW_BYTE(PROFINET_DEFAULT_API);
	pDataRecord->APILowWordHighByte  = _PN_U32_LOW_HIGH_BYTE(PROFINET_DEFAULT_API);
	pDataRecord->APILowWordLowByte   = _PN_U32_LOW_LOW_BYTE(PROFINET_DEFAULT_API);
	pDataRecord->IndexHighByte = _PN_U16_HIGH_BYTE(dev_data->slot_data[ii]->subslot_data[jj]->data_record[kk]->index);
	pDataRecord->IndexLowByte  = _PN_U16_LOW_BYTE(dev_data->slot_data[ii]->subslot_data[jj]->data_record[kk]->index);
	pDataRecord->LengthHighByte = _PN_U16_HIGH_BYTE(dev_data->slot_data[ii]->subslot_data[jj]->data_record[kk]->data_length);
	pDataRecord->LengthLowByte  = _PN_U16_LOW_BYTE(dev_data->slot_data[ii]->subslot_data[jj]->data_record[kk]->data_length);

	pData = (char *) (pDataRecord + 1);
	memcpy(pData, dev_data->slot_data[ii]->subslot_data[jj]->data_record[kk]->data, dev_data->slot_data[ii]->subslot_data[jj]->data_record[kk]->data_length);
	pData += dev_data->slot_data[ii]->subslot_data[jj]->data_record[kk]->data_length;
	pDataRecord = (T_PN_DATA_RECORD *) pData;
      }
    }
  }

  /*  if (device_ref != 0) {

  pData = (char *) (pSDR);
  printf("Download of device: %s\n", dev_data->device_name);
  printf("Total datalength %d\n\n", length);
  for (ii = 0; ii < length; ii++) {
  if (ii % 16 == 0) printf("\n");
  printf("%02hhX ", pData[ii]);
  }
  printf("\n");
  printf("\n");
    } */
}

int unpack_get_los_con(T_PNAK_SERVICE_DESCRIPTION* pSdb, io_sAgentLocal *local)
{

  if (pSdb->Result == PNAK_RESULT_POS) {
    T_PN_SERVICE_GET_LIST_OF_STATION_CON*    pGetLOSCon;
    T_PN_DEVICE_INFO*                        pDeviceInfo;
    PnDeviceInfo                             *dev_info;

	
    PN_U16                        NumberDevices;
    unsigned short                ii, name_length;

    
    pGetLOSCon     = (T_PN_SERVICE_GET_LIST_OF_STATION_CON *) (pSdb + 1);
    pDeviceInfo    = (T_PN_DEVICE_INFO *) (pGetLOSCon + 1);

    NumberDevices = _HIGH_LOW_BYTES_TO_PN_U16 (pGetLOSCon->NumberOfDevicesHighByte, pGetLOSCon->NumberOfDevicesLowByte);

    /* Find configured device */

    if (NumberDevices == 0) printf("0\r\n");

    for (ii = 0; ii < NumberDevices; ii++) {

      dev_info = new PnDeviceInfo;

      dev_info->ipaddress[3] = pDeviceInfo->Ip.AddressHighWordHighByte;
      dev_info->ipaddress[2] = pDeviceInfo->Ip.AddressHighWordLowByte;
      dev_info->ipaddress[1] = pDeviceInfo->Ip.AddressLowWordHighByte;
      dev_info->ipaddress[0] = pDeviceInfo->Ip.AddressLowWordLowByte;

      dev_info->macaddress[5] = pDeviceInfo->MacAddress.HighAndVersionHighByte;
      dev_info->macaddress[4] = pDeviceInfo->MacAddress.HighAndVersionLowByte;
      dev_info->macaddress[3] = pDeviceInfo->MacAddress.MidHighByte;
      dev_info->macaddress[2] = pDeviceInfo->MacAddress.MidLowByte;
      dev_info->macaddress[1] = pDeviceInfo->MacAddress.LowHighByte;
      dev_info->macaddress[0] = pDeviceInfo->MacAddress.LowLowByte;

      dev_info->deviceid = _HIGH_LOW_BYTES_TO_PN_U16(pDeviceInfo->Property.DeviceIdHighByte, pDeviceInfo->Property.DeviceIdLowByte);
      dev_info->vendorid = _HIGH_LOW_BYTES_TO_PN_U16(pDeviceInfo->Property.VendorIdHighByte, pDeviceInfo->Property.VendorIdLowByte);

      name_length = _HIGH_LOW_BYTES_TO_PN_U16(pDeviceInfo->DeviceNameLengthHighByte, pDeviceInfo->DeviceNameLengthLowByte);

      printf("no: %d            mac: %hhx:%hhx:%hhx:%hhx:%hhx:%hhx\r\n", NumberDevices, 
	     pDeviceInfo->MacAddress.HighAndVersionHighByte, pDeviceInfo->MacAddress.HighAndVersionLowByte,
	     pDeviceInfo->MacAddress.MidHighByte, pDeviceInfo->MacAddress.MidLowByte,
	     pDeviceInfo->MacAddress.LowHighByte, pDeviceInfo->MacAddress.LowLowByte);

      pDeviceInfo++;

      memset(dev_info->devname, 0, sizeof(dev_info->devname));
      strncpy(dev_info->devname, (char *) pDeviceInfo, name_length);

      local->dev_info.push_back(dev_info);

      pDeviceInfo = (T_PN_DEVICE_INFO *)((unsigned char *) pDeviceInfo + name_length);

    }	
    return PNAK_OK;
  }
  else if (pSdb->Result == PNAK_RESULT_NEG) {
    T_PN_SERVICE_ERROR_CON*   pErrorCon = (T_PN_SERVICE_ERROR_CON*) (pSdb + 1);


    printf(
	   "channel %d: get_los.con [-] (%d)\r\n"
	   "            code       : %d (0x%02x)\r\n"
	   "            detail     : %d (0x%02x)\r\n"
	   "            add. detail: %d (0x%02x)\r\n"
	   "            area       : %d (0x%02x)\r\n",
	   0,
	   pSdb->DeviceRef,
	   pErrorCon->Code, pErrorCon->Code,
	   pErrorCon->Detail, pErrorCon->Detail,
	   pErrorCon->AdditionalDetail, pErrorCon->AdditionalDetail,
	   pErrorCon->AreaCode, pErrorCon->AreaCode
	   );
  }

  return -1;
}

int unpack_get_alarm_con(T_PNAK_SERVICE_DESCRIPTION* pSdb, io_sAgentLocal *local, io_sAgent *ap)
{

  if (pSdb->Result == PNAK_RESULT_POS) {
    T_PN_SERVICE_GET_ALARM_CON*    pGAC;

    unsigned short alarm_type;
    unsigned short alarm_prio;
    unsigned short alarm_ref, ii, jj;
    unsigned short rem_alarms;
    unsigned int   slot_number;
    unsigned int   sub_slot_number;
    unsigned int   module_ident_number;
    unsigned int   submodule_ident_number;
    unsigned short alarm_spec;
    unsigned short  data_length;
    unsigned char  *data;
    PnDeviceData   *device;;

    pGAC     = (T_PN_SERVICE_GET_ALARM_CON *) (pSdb + 1);


    alarm_prio = pGAC->AlarmPriority;
    rem_alarms = pGAC->RemainingAlarms;

    alarm_ref = _HIGH_LOW_BYTES_TO_PN_U16(pGAC->AlarmRefHighByte, pGAC->AlarmRefLowByte);
    alarm_type = _HIGH_LOW_BYTES_TO_PN_U16(pGAC->AlarmTypeHighByte, pGAC->AlarmTypeLowByte);
    slot_number = _HIGH_LOW_BYTES_TO_PN_U16(pGAC->SlotNumberHighByte, pGAC->SlotNumberLowByte);
    sub_slot_number = _HIGH_LOW_BYTES_TO_PN_U16(pGAC->SubSlotNumberHighByte, pGAC->SubSlotNumberLowByte);
    module_ident_number = _HIGH_LOW_BYTES_TO_PN_U32(pGAC->ModuleIdentNumberHighWordHighByte, 
						    pGAC->ModuleIdentNumberHighWordLowByte, 
						    pGAC->ModuleIdentNumberLowWordHighByte, 
						    pGAC->ModuleIdentNumberLowWordLowByte);
    submodule_ident_number = _HIGH_LOW_BYTES_TO_PN_U32(pGAC->SubmoduleIdentNumberHighWordHighByte, 
						    pGAC->SubmoduleIdentNumberHighWordLowByte, 
						    pGAC->SubmoduleIdentNumberLowWordHighByte, 
						    pGAC->SubmoduleIdentNumberLowWordLowByte);
    alarm_spec = _HIGH_LOW_BYTES_TO_PN_U16(pGAC->SpecifierHighByte, pGAC->SpecifierLowByte);
    data_length = _HIGH_LOW_BYTES_TO_PN_U16(pGAC->LengthHighByte, pGAC->LengthLowByte);
    data = (unsigned char *)(pGAC + 1);

    /* Find the device */

    for (ii = 0; ii < local->device_data.size(); ii++) {
      if (local->device_data[ii]->alarm_ref == alarm_ref) {
	device = local->device_data[ii];

	device->alarm_data.alarm_type             = alarm_type;
	device->alarm_data.alarm_prio             = alarm_prio;
	device->alarm_data.rem_alarms             = rem_alarms;
	device->alarm_data.slot_number            = slot_number;
	device->alarm_data.sub_slot_number        = sub_slot_number;
	device->alarm_data.module_ident_number    = module_ident_number;
	device->alarm_data.submodule_ident_number = submodule_ident_number;
	device->alarm_data.alarm_spec             = alarm_spec;
	device->alarm_data.data_length            = data_length;

	break;
      }
    }

    if (ap) {
      /* Find corresponding device */
      io_sRack *slave_list;
      for (slave_list = ap->racklist, jj = 0; 
	   (slave_list != NULL) && jj < ii - 1;
	   slave_list = slave_list->next, jj++) {}
      if (slave_list) {
	pwr_sClass_PnDevice *dev;
	dev = (pwr_sClass_PnDevice *) slave_list->op;

	dev->Alarm.Type = alarm_type;
	dev->Alarm.Prio = alarm_prio;
	dev->Alarm.Remaining = rem_alarms;
	dev->Alarm.SlotNumber = slot_number;
	dev->Alarm.SubslotNumber = sub_slot_number;
	dev->Alarm.ModuleIdentNumber = module_ident_number;
	dev->Alarm.SubmoduleIdentNumber = submodule_ident_number;
	dev->Alarm.Specifier = alarm_spec;
	dev->Alarm.ManuSpecLength = data_length;
	if (data_length > 0) {
	  memcpy(dev->Alarm.Data, data, MIN(data_length, sizeof(dev->Alarm.Data)));
	}

      }
    }

    printf(
	   "Alarm prio           %d\r\n"
	   "      remaining      %d\r\n"
	   "      type           %d\r\n"
	   "      slot           %d\r\n"
	   "      subslot        %d\r\n"
	   "      module_id      %d\r\n"
	   "      submodule_id   %d\r\n"
	   "      spec           %d\r\n",
	   alarm_prio,
	   rem_alarms,
	   alarm_type,
	   slot_number,
	   sub_slot_number,
	   module_ident_number,
	   submodule_ident_number,
	   alarm_spec);
    return PNAK_OK;
  }
  else if (pSdb->Result == PNAK_RESULT_NEG) {
    T_PN_SERVICE_ERROR_CON*   pErrorCon = (T_PN_SERVICE_ERROR_CON*) (pSdb + 1);


    printf(
	   "channel %d: get_los.con [-] (%d)\r\n"
	   "            code       : %d (0x%02x)\r\n"
	   "            detail     : %d (0x%02x)\r\n"
	   "            add. detail: %d (0x%02x)\r\n"
	   "            area       : %d (0x%02x)\r\n",
	   0,
	   pSdb->DeviceRef,
	   pErrorCon->Code, pErrorCon->Code,
	   pErrorCon->Detail, pErrorCon->Detail,
	   pErrorCon->AdditionalDetail, pErrorCon->AdditionalDetail,
	   pErrorCon->AreaCode, pErrorCon->AreaCode
	   );
  }

  return -1;
}
int unpack_get_device_state_con(T_PNAK_SERVICE_DESCRIPTION* pSdb, io_sAgentLocal *local, io_sAgent *ap)
{

  if (pSdb->Result == PNAK_RESULT_POS) {
    T_PN_SERVICE_GET_DEVICE_STATE_CON*    pGDSC;
    T_PN_DIFF_MODULE*                     pDiffModule;
	
    PN_U16                        no_diff_modules;
    PN_U16                        diff_mod_index;
    unsigned short                device_ref, ii, jj;

    PnDeviceData                  *device;
    unsigned short                save_first = TRUE;
    unsigned short                err_slot_number;
    unsigned short                err_module_state;
    unsigned short                phys_ident_number;
    unsigned short                dev_ind;

    
    device_ref = pSdb->DeviceRef;

    /* Find configured device */

    for (ii = 0; ii < local->device_data.size(); ii++) {
      if (local->device_data[ii]->device_ref == device_ref) {
	device = local->device_data[ii];
        break;
      }
    }

    if (ii == local->device_data.size()) return -1; // ERR_NODEV_FOUND;

    dev_ind = ii;
	
    pGDSC            = (T_PN_SERVICE_GET_DEVICE_STATE_CON *) (pSdb + 1);
    pDiffModule      = (T_PN_DIFF_MODULE *) (pGDSC + 1);
    no_diff_modules  = _HIGH_LOW_BYTES_TO_PN_U16 (pGDSC->NumberOfDiffModulesHighByte, pGDSC->NumberOfDiffModulesLowByte);
    
    device->no_diff_modules = no_diff_modules;
    device->device_state = _HIGH_LOW_BYTES_TO_PN_U16(pGDSC->StateHighByte, pGDSC->StateLowByte);

    //    printf("No diff modules: %d \r\n", no_diff_modules);
	
    for (diff_mod_index = 0u; diff_mod_index < no_diff_modules; diff_mod_index++) {
      T_PN_DIFF_MODULE_API  *pDiffModuleAPI = (T_PN_DIFF_MODULE_API *) (pDiffModule + 1);
      PN_U16            no_apis;
      PN_U16            api_ind;
      

      no_apis = _HIGH_LOW_BYTES_TO_PN_U16 (pDiffModule->NumberOfAPIsHighByte, pDiffModule->NumberOfAPIsLowByte);

      for (api_ind = 0u; api_ind < no_apis; api_ind++) {
	T_PN_DIFF_MODULE_SLOT    *pModuleSlot = (T_PN_DIFF_MODULE_SLOT *) (pDiffModuleAPI + 1);
	
	PN_U16                    no_slots;
	PN_U16                    slot_ind;
	
	    
	no_slots = _HIGH_LOW_BYTES_TO_PN_U16 (pDiffModuleAPI->NumberOfModulesHighByte, pDiffModuleAPI->NumberOfModulesLowByte);
	
	for (slot_ind = 0u; slot_ind < no_slots; slot_ind++) {
	  T_PN_DIFF_MODULE_SUBSLOT    *pModuleSubSlot = (T_PN_DIFF_MODULE_SUBSLOT *) (pModuleSlot + 1);

	  PN_U16                    no_subslots;
	  PN_U16                    subslot_ind;
	  PnModuleData              *module_data;

	  no_subslots = _HIGH_LOW_BYTES_TO_PN_U16 (pModuleSlot->NumberOfSubmodulesHighByte, pModuleSlot->NumberOfSubmodulesLowByte);


	  for (ii = 0; ii < device->module_data.size(); ii++) {
            
            module_data = device->module_data[ii];

            if (module_data->slot_number == _HIGH_LOW_BYTES_TO_PN_U16 (pModuleSlot->SlotNumberHighByte, pModuleSlot->SlotNumberLowByte)) {
	      module_data->state = _HIGH_LOW_BYTES_TO_PN_U16 (pModuleSlot->StateHighByte, pModuleSlot->StateLowByte);
	      module_data->phys_ident_number = _HIGH_LOW_BYTES_TO_PN_U32(pModuleSlot->IdentNumberHighWordHighByte, 
									 pModuleSlot->IdentNumberHighWordLowByte, 
									 pModuleSlot->IdentNumberLowWordHighByte, 
									 pModuleSlot->IdentNumberLowWordLowByte);
	      //	      printf("  Slot no: %d, State: %d \r\n", module_data->slot_number, module_data->state);
	      if (save_first) {
		err_slot_number = module_data->slot_number;
		err_module_state = module_data->state;
		phys_ident_number = module_data->phys_ident_number;
		save_first = FALSE;
	      }
	      break;
	    }
	  }

	  for (subslot_ind = 0; subslot_ind < no_subslots; subslot_ind++) {

	    PnSubmoduleData *submodule_data;
            
	    if (ii < device->module_data.size()) {
	      for (jj = 0; jj < module_data->submodule_data.size(); jj++) {
		
		submodule_data = module_data->submodule_data[jj];
		if (submodule_data->subslot_number == _HIGH_LOW_BYTES_TO_PN_U16 (pModuleSubSlot->SubSlotNumberHighByte, pModuleSubSlot->SubSlotNumberLowByte)) {
		    submodule_data->state = _HIGH_LOW_BYTES_TO_PN_U16 (pModuleSubSlot->StateHighByte, pModuleSubSlot->StateLowByte);
		    submodule_data->phys_ident_number = _HIGH_LOW_BYTES_TO_PN_U32(pModuleSubSlot->IdentNumberHighWordHighByte, 
									       pModuleSubSlot->IdentNumberHighWordLowByte, 
									       pModuleSubSlot->IdentNumberLowWordHighByte, 
									       pModuleSubSlot->IdentNumberLowWordLowByte);

		    //		    printf("    SubSlot no: %d, State: %d \r\n", submodule_data->subslot_number, submodule_data->state);

		}
	      }
	    }
	    pModuleSubSlot++;
          }
	  pModuleSlot = (T_PN_DIFF_MODULE_SLOT *) pModuleSubSlot;
	}	
	pDiffModuleAPI = (T_PN_DIFF_MODULE_API *) pModuleSlot;    
      }
      pDiffModule = (T_PN_DIFF_MODULE *) pDiffModuleAPI;	  
    }

    if (ap) {
      /* Find corresponding device */
      io_sRack *slave_list;
      for (slave_list = ap->racklist, jj = 0; 
	   (slave_list != NULL) && jj < dev_ind - 1;
	   slave_list = slave_list->next, jj++) {}
      if (slave_list) {
	pwr_sClass_PnDevice *dev;
	dev = (pwr_sClass_PnDevice *) slave_list->op;
	dev->NoDiffModules = no_diff_modules;
	if (device->device_state == PNAK_DEVICE_STATE_CONNECTED)
	  dev->Status = PB__NORMAL;
	else 
	  dev->Status = PB__NOCONN;
	if(!save_first) {
	  dev->ErrSlotNumber = err_slot_number;
	  dev->ErrModuleState = err_module_state;
	  dev->PhysIdentNumber = phys_ident_number;
	} else {
	  dev->ErrSlotNumber = 0;
	  dev->ErrModuleState = 0;
	  dev->PhysIdentNumber = 0;
	}
      }
    }

    return PNAK_OK;
  }
  else if (pSdb->Result == PNAK_RESULT_NEG) {
    T_PN_SERVICE_ERROR_CON*   pErrorCon = (T_PN_SERVICE_ERROR_CON*) (pSdb + 1);


    printf(
	   "channel %d: download.con [-] (%d)\r\n"
	   "            code       : %d (0x%02x)\r\n"
	   "            detail     : %d (0x%02x)\r\n"
	   "            add. detail: %d (0x%02x)\r\n"
	   "            area       : %d (0x%02x)\r\n",
	   0,
	   pSdb->DeviceRef,
	   pErrorCon->Code, pErrorCon->Code,
	   pErrorCon->Detail, pErrorCon->Detail,
	   pErrorCon->AdditionalDetail, pErrorCon->AdditionalDetail,
	   pErrorCon->AreaCode, pErrorCon->AreaCode
	   );
  }
  
  return -1;
  
}

int unpack_download_con(T_PNAK_SERVICE_DESCRIPTION* pSdb, io_sAgentLocal *local)
{

  if (pSdb->Result == PNAK_RESULT_POS) {
    T_PN_SERVICE_DOWNLOAD_CON*    pDownloadCon;
    T_PN_IOCR_INFO*               pIOCRInfo;
	
    PN_U16                        NumberIOCRs;
    PN_U16                        IOCRIndex;
    unsigned short                device_ref, ii, jj;

    PnDeviceData                  *device;
    
    device_ref = pSdb->DeviceRef;

    /* Find configured device */

    for (ii = 0; ii < local->device_data.size(); ii++) {
      if (local->device_data[ii]->device_ref == device_ref) {
	device = local->device_data[ii];
        break;
      }
    }

    if (ii == local->device_data.size()) return -1; // ERR_NODEV_FOUND;
	
    pDownloadCon = (T_PN_SERVICE_DOWNLOAD_CON*) (pSdb + 1);
    pIOCRInfo    = (T_PN_IOCR_INFO*) (pDownloadCon + 1);
    NumberIOCRs  = _HIGH_LOW_BYTES_TO_PN_U16 (pDownloadCon->NumberOfIOCRHighByte, pDownloadCon->NumberOfIOCRLowByte);
	
    device->alarm_ref = _HIGH_LOW_BYTES_TO_PN_U16(pDownloadCon->AlarmRefHighByte, pDownloadCon->AlarmRefLowByte);
	
    for (IOCRIndex = 0u; IOCRIndex < NumberIOCRs; IOCRIndex++) {
      T_PN_API_INFO*    pAPIInfo = (T_PN_API_INFO*) (pIOCRInfo + 1);
      PN_U16            NumberAPIs;
      PN_U16            APIIndex;
      PnIOCRData       *iocr_data;
      
      unsigned short    type;
      
      type = _HIGH_LOW_BYTES_TO_PN_U16(pIOCRInfo->TypeHighByte, pIOCRInfo->TypeLowByte);
      
      for (ii = 0; ii < device->iocr_data.size(); ii++) {
	
	if (device->iocr_data[ii]->type == type) {
	  iocr_data = device->iocr_data[ii];
          break;
	}
      }

      if (ii == device->iocr_data.size()) {
        /* This iocr is not found, log some thing and continue */

        printf("iocr not found %d \n", type);
        continue;
      }
      
      iocr_data->type = _HIGH_LOW_BYTES_TO_PN_U16 (pIOCRInfo->TypeHighByte, pIOCRInfo->TypeLowByte);
      iocr_data->identifier = _HIGH_LOW_BYTES_TO_PN_U16 (pIOCRInfo->IOCRIdentifierHighByte, pIOCRInfo->IOCRIdentifierLowByte);
      iocr_data->io_data_length = _HIGH_LOW_BYTES_TO_PN_U16 (pIOCRInfo->IODataLengthHighByte, pIOCRInfo->IODataLengthLowByte);
      iocr_data->io_data = (unsigned char *) calloc(1, iocr_data->io_data_length);	  
      //      printf("            iocr (0x%04x) 0x%04x %d\r\n",
      //	     _HIGH_LOW_BYTES_TO_PN_U16 (pIOCRInfo->TypeHighByte, pIOCRInfo->TypeLowByte),
      //	     _HIGH_LOW_BYTES_TO_PN_U16 (pIOCRInfo->IOCRIdentifierHighByte, pIOCRInfo->IOCRIdentifierLowByte),
      //	     _HIGH_LOW_BYTES_TO_PN_U16 (pIOCRInfo->IODataLengthHighByte, pIOCRInfo->IODataLengthLowByte));
      
      NumberAPIs = _HIGH_LOW_BYTES_TO_PN_U16 (pIOCRInfo->NumberOfAPIsHighByte, pIOCRInfo->NumberOfAPIsLowByte);
      
      for (APIIndex = 0u; APIIndex < NumberAPIs; APIIndex++) {
	T_PN_DATA_INFO*   pDataInfo = (T_PN_DATA_INFO*) (pAPIInfo + 1);
	
	PN_U16            NumberIODatas;
	PN_U16            IODataIndex;
	
	    
	//	printf("              api 0x%04x%04x\r\n"
	//	       "                data\r\n",
	//	       _HIGH_LOW_BYTES_TO_PN_U16 (pAPIInfo->APIHighWordHighByte, pAPIInfo->APIHighWordLowByte),
	//	       _HIGH_LOW_BYTES_TO_PN_U16 (pAPIInfo->APILowWordHighByte, pAPIInfo->APILowWordLowByte));
	//
	
	NumberIODatas = _HIGH_LOW_BYTES_TO_PN_U16 (pAPIInfo->NumberOfIODataHighByte, pAPIInfo->NumberOfIODataLowByte);
	
	for (IODataIndex = 0u; IODataIndex < NumberIODatas; IODataIndex++) {
	  //	  printf("                  slot: %d subslot: %d offset: %d\r\n",
	  //		 _HIGH_LOW_BYTES_TO_PN_U16 (pDataInfo->SlotNumberHighByte, pDataInfo->SlotNumberLowByte),
	  //		 _HIGH_LOW_BYTES_TO_PN_U16 (pDataInfo->SubSlotNumberHighByte, pDataInfo->SubSlotNumberLowByte),
	  //		 _HIGH_LOW_BYTES_TO_PN_U16 (pDataInfo->OffsetHighByte, pDataInfo->OffsetLowByte));
	  
	  for (ii = 0; ii < device->module_data.size(); ii++) {
            PnModuleData *module_data;
            
            module_data = device->module_data[ii];

            if (module_data->slot_number == _HIGH_LOW_BYTES_TO_PN_U16 (pDataInfo->SlotNumberHighByte, pDataInfo->SlotNumberLowByte)) {
	      for (jj = 0; jj < module_data->submodule_data.size(); jj++) {
		PnSubmoduleData *submodule_data;
		
		submodule_data = module_data->submodule_data[jj];
		if (submodule_data->subslot_number == _HIGH_LOW_BYTES_TO_PN_U16 (pDataInfo->SubSlotNumberHighByte, pDataInfo->SubSlotNumberLowByte)) {
		  if (PROFINET_IO_CR_TYPE_INPUT == type) {
		    submodule_data->offset_io_in = _HIGH_LOW_BYTES_TO_PN_U16 (pDataInfo->OffsetHighByte, pDataInfo->OffsetLowByte);
		  } else {
		    submodule_data->offset_io_out = _HIGH_LOW_BYTES_TO_PN_U16 (pDataInfo->OffsetHighByte, pDataInfo->OffsetLowByte);
		  }
		}
	      }
	    }
          }
	  pDataInfo++;
	}
	
	//	printf ("                status\r\n");
	NumberIODatas = _HIGH_LOW_BYTES_TO_PN_U16 (pAPIInfo->NumberOfIOStatusHighByte, pAPIInfo->NumberOfIOStatusLowByte);
	
	for (IODataIndex = 0u; IODataIndex < NumberIODatas; IODataIndex++) {
	  //	  printf("                  slot: %d subslot: %d offset: %d\r\n",
	  //		 _HIGH_LOW_BYTES_TO_PN_U16 (pDataInfo->SlotNumberHighByte, pDataInfo->SlotNumberLowByte),
	  //		 _HIGH_LOW_BYTES_TO_PN_U16 (pDataInfo->SubSlotNumberHighByte, pDataInfo->SubSlotNumberLowByte),
	  //		 _HIGH_LOW_BYTES_TO_PN_U16 (pDataInfo->OffsetHighByte, pDataInfo->OffsetLowByte));
	  
	  for (ii = 0; ii < device->module_data.size(); ii++) {
            PnModuleData *module_data;
            
            module_data = device->module_data[ii];

            if (module_data->slot_number == _HIGH_LOW_BYTES_TO_PN_U16 (pDataInfo->SlotNumberHighByte, pDataInfo->SlotNumberLowByte)) {
	      for (jj = 0; jj < module_data->submodule_data.size(); jj++) {
		PnSubmoduleData *submodule_data;
            
		submodule_data = module_data->submodule_data[jj];
		if (submodule_data->subslot_number == _HIGH_LOW_BYTES_TO_PN_U16 (pDataInfo->SubSlotNumberHighByte, pDataInfo->SubSlotNumberLowByte)) {
		  if (PROFINET_IO_CR_TYPE_INPUT == type) {
		    submodule_data->offset_status_in = _HIGH_LOW_BYTES_TO_PN_U16 (pDataInfo->OffsetHighByte, pDataInfo->OffsetLowByte);
		  } else {
		    submodule_data->offset_status_out = _HIGH_LOW_BYTES_TO_PN_U16 (pDataInfo->OffsetHighByte, pDataInfo->OffsetLowByte);
		  }
		}
	      }
	    }
          }
	  pDataInfo++;
	}
	    
	pAPIInfo = (T_PN_API_INFO*) pDataInfo;
      }
	  
      pIOCRInfo = (T_PN_IOCR_INFO*) pAPIInfo;
    }

    return PNAK_OK;
  }
  else if (pSdb->Result == PNAK_RESULT_NEG) {
    T_PN_SERVICE_ERROR_CON*   pErrorCon = (T_PN_SERVICE_ERROR_CON*) (pSdb + 1);


    printf(
	   "channel %d: download.con [-] (%d)\r\n"
	   "            code       : %d (0x%02x)\r\n"
	   "            detail     : %d (0x%02x)\r\n"
	   "            add. detail: %d (0x%02x)\r\n"
	   "            area       : %d (0x%02x)\r\n",
	   0,
	   pSdb->DeviceRef,
	   pErrorCon->Code, pErrorCon->Code,
	   pErrorCon->Detail, pErrorCon->Detail,
	   pErrorCon->AdditionalDetail, pErrorCon->AdditionalDetail,
	   pErrorCon->AreaCode, pErrorCon->AreaCode
	   );
  }

  return -1;
  
}

int handle_service_con(io_sAgentLocal *local, io_sAgent *ap)
{

  T_PNAK_WAIT_OBJECT           wait_object;
  int                          sts;
  unsigned short               ii;

  wait_object = PNAK_WAIT_OBJECT_SERVICE_CON;

  sts = pnak_wait_for_multiple_objects(0, &wait_object, PNAK_INFINITE_TIMEOUT);

  if (sts == PNAK_OK) {

    memset(&local->service_con, 0, sizeof(T_PNAK_SERVICE_CON));
    sts = pnak_get_service_con(0, &local->service_con);

    if (sts == PNAK_NOTIFICATION_RECEIVED) {
      for (ii = 0; ii < local->service_con.NumberEntries; ii++) {
	T_PNAK_SERVICE_DESCRIPTION*   pSdb;
	unsigned int                  offset;

	offset = local->service_con.ServiceEntry[ii].ServiceOffset;
	pSdb = (T_PNAK_SERVICE_DESCRIPTION *) &local->service_con.ServiceChannel[offset];

	if ((pSdb->Instance == PN) || (pSdb->Instance == PN_CONTROLLER)) {
	  switch (pSdb->Service) {
	  case PN_SERVICE_DOWNLOAD          : {
            sts = unpack_download_con(pSdb, local);
	    break;
	  }

	  case PN_SERVICE_SET_IDENTIFICATION:
	  case PN_SERVICE_READ              :
	  case PN_SERVICE_WRITE             : {
	    break;
	  }

	  case PN_SERVICE_GET_ALARM       : {
	    sts = unpack_get_alarm_con(pSdb, local, ap);
	    break;
	  }
	  
	  case PN_SERVICE_GET_DEVICE_STATE: {
	    sts = unpack_get_device_state_con(pSdb, local, ap);
	    break;
	  }
	  case PN_SERVICE_ALARM_ACK       : {
	    break;
	  }

	  
	  default: {
	    printf("channel %d: unhandled service confirmation [0x%x]\r\n",
		   0,
		   pSdb->Service
		   );
	  }
	  }
	}
	else if (pSdb->Instance == PN_SUPERVISOR) {
	  switch (pSdb->Service) {

	  case PN_SERVICE_GET_LIST_OF_STATION: {
            sts = unpack_get_los_con(pSdb, local);
	    break;
	  }

	  case PN_SERVICE_SET_IDENTIFICATION:
	  case PN_SERVICE_SET_IP_SETTINGS   :
	  case PN_SERVICE_SET_DEVICE_NAME   : {
	    if (pSdb->Result == PNAK_RESULT_NEG) {
	      T_PN_SERVICE_ERROR_CON*   pErrorCon = (T_PN_SERVICE_ERROR_CON*) (pSdb + 1);


	      printf(
		     "channel %d: get_los.con [-] (%d)\r\n"
		     "            code       : %d (0x%02x)\r\n"
		     "            detail     : %d (0x%02x)\r\n"
		     "            add. detail: %d (0x%02x)\r\n"
		     "            area       : %d (0x%02x)\r\n",
		     0,
		     pSdb->DeviceRef,
		     pErrorCon->Code, pErrorCon->Code,
		     pErrorCon->Detail, pErrorCon->Detail,
		     pErrorCon->AdditionalDetail, pErrorCon->AdditionalDetail,
		     pErrorCon->AreaCode, pErrorCon->AreaCode
		     );
	    }
	    break;
	  }

	  default: {
	    printf("channel %d: unhandled service confirmation [0x%x]\r\n",
		   0,
		   pSdb->Service
		   );
	  }
	  }
	}
      }
    }
  }

  return sts;
}


void handle_exception (io_sAgentLocal *local) {
  return;
}

void handle_state_changed (io_sAgentLocal *local) {
  return;
}


void handle_device_state_changed (io_sAgentLocal *local, io_sAgent *ap) {
  int                          sts;
  unsigned short               ii, jj;
  T_PNAK_EVENT_DEVICE_STATE    dev_state;
  
  sts = pnak_get_device_state_ind(0, &dev_state);

  if (sts == PNAK_NOTIFICATION_RECEIVED) {
  /* Check state for all devices */

    for (ii = 1; ii < local->device_data.size(); ii++) {
      //      printf("Dev_ref %d, State, %d \r\n", ii, dev_state.State[ii]); 

      //  for (ii = 0; ii < 1; ii++) {
      if (dev_state.State[ii] != local->device_data[ii]->device_state) {
	local->device_data[ii]->device_state = dev_state.State[ii];

	if (ap) {
	  /* Find corresponding device */
	  io_sRack *slave_list;
	  for (slave_list = ap->racklist, jj = 0; 
	       (slave_list != NULL) && jj < ii - 1;
	       slave_list = slave_list->next, jj++) {}
	  if (slave_list) {
	    pwr_sClass_PnDevice *dev;
	    dev = (pwr_sClass_PnDevice *) slave_list->op;
	    dev->State = dev_state.State[ii];
	    errh_Info( "Profinet - New device state, dev: %s, state: %d", slave_list->Name, dev->State);
	  }
	}

	if ( dev_state.State[ii] == PNAK_DEVICE_STATE_CONNECTED) {
	  pack_get_device_state_req(&local->service_req_res, local->device_data[ii]->device_ref);
	
	  sts = pnak_send_service_req_res(0, &local->service_req_res);
      
	  if (sts == PNAK_OK) {
	    sts = handle_service_con(local, ap);
	  }
	}
      }
    }
  }

  return;

}


void handle_alarm_indication (io_sAgentLocal *local, io_sAgent *ap) {
  T_PNAK_EVENT_ALARM           pAlarm;
  int                          sts;
  unsigned short               ii, index, bit_no;

  sts = pnak_get_alarm_ind(0, &pAlarm);

  if (sts == PNAK_NOTIFICATION_RECEIVED) {

    for (ii = 0; ii < local->device_data.size(); ii++) {
      index = ii / 8;
      bit_no = ii % 8;
      if (pAlarm.DeviceReference[index] & (1 << bit_no)) {
	printf("New alarm for device: %d\r\n", ii);

	pack_get_alarm_req(&local->service_req_res, local->device_data[ii]->alarm_ref, local->device_data[ii]->device_ref);

	sts = pnak_send_service_req_res(0, &local->service_req_res);

	if (sts == PNAK_OK) {
	  sts = handle_service_con(local, ap);
	  if (sts == PNAK_OK) {
	    pack_alarm_ack_req(&local->service_req_res, local->device_data[ii]->alarm_ref, local->device_data[ii]->alarm_data.alarm_prio, local->device_data[ii]->device_ref);

	    sts = pnak_send_service_req_res(0, &local->service_req_res);

	    if (sts == PNAK_OK) {
	      sts = handle_service_con(local, ap);
	    }
	  }
	}

	break;
      }
    }    
  }
}

/* Supervision thread */

void *handle_events(void *ptr) {
  agent_args           *args;
  io_sAgentLocal       *local;
  io_sAgent            *ap;
  T_PNAK_WAIT_OBJECT           wait_object;
  int                          sts;

  args = (agent_args *) ptr;
  local = (io_sAgentLocal *) args->local;
  ap = args->ap;


  /* Do forever ... */

  while (1) {
    wait_object = PNAK_WAIT_OBJECTS_EVENT_IND | PNAK_WAIT_OBJECTS_OTHER;
    sts = pnak_wait_for_multiple_objects(0, &wait_object, PNAK_INFINITE_TIMEOUT);
    
    if (sts == PNAK_OK) {
      if (wait_object & PNAK_WAIT_OBJECT_EXCEPTION) {
	//	printf("Exception !!\n"); 
	handle_exception (local);
      }
      
      if (wait_object & PNAK_WAIT_OBJECT_STATE_CHANGED) {
	//	printf("State changed !!"); 
	handle_state_changed (local);
      }
      
      if (wait_object & PNAK_WAIT_OBJECT_DEVICE_STATE_CHANGED) {
	handle_device_state_changed (local, ap);
      }
      
      if (wait_object & PNAK_WAIT_OBJECT_ALARM) {
	//	printf("Alarm !!\n"); 
	handle_alarm_indication (local, ap);
      }
      
      if (wait_object & PNAK_WAIT_OBJECT_CHANNEL_CLOSED) {
	//	printf("Channel closed !!"); 
	//     What to do if channel closes ???;
      }
      
      if (wait_object & PNAK_WAIT_OBJECT_ETHERNET_STATE_CHANGED) {
	//	printf("Ethernet state changed !!"); 
	//     What to do if ethernet state changes ???;
      }
      
      if (wait_object & PNAK_WAIT_OBJECT_INTERRUPTED) {
	//	printf("Interrupted !!"); 
	//     What to do if interrupted ???;
      }
      
    }
    else if ( (sts == PNAK_ERR_FATAL_ERROR ) ||
	      (sts == PNAK_EXCEPTION_THROWN) ) {
	printf("Err Fatal / Exception !!"); 
      //    user_handle_exception (ChannelId);
    }
    else {
	printf("Running == NOT !!"); 
      //    pThisSmObject->Running = PN_FALSE;
    }
  }
}
