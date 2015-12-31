#include "MidiQueue.h"

#include "../Common/STRINGS.h"	// TODO: Do we like this?
#include "MIDIController.h"

// Pending MIDI events
GTAR_MIDI_EVENT m_gTarPendingMidiEvents[MAX_PENDING_EVENTS];

int32_t  m_gTarPendingMidiEvents_n = 0;	// number of events queued
int32_t  m_gTarPendingMidiEvents_c = 0;	// event cursor (to add events)
int32_t  m_gTarPendingMidiEvents_e = 0;	// event execute

RESULT InitializeMIDIQueue() {
	RESULT r = R_OK;

	// Pending events
	memset(m_gTarPendingMidiEvents, 0, sizeof(m_gTarPendingMidiEvents));

	m_gTarPendingMidiEvents_n = 0;	// number of events queued
	m_gTarPendingMidiEvents_c = 0;	// event cursor (to add events)
	m_gTarPendingMidiEvents_e = 0;	// event execute

	DEBUG_LINEOUT_NA("MIDI Queue initialized!");

Error:
	return r;
}

RESULT QueueNewMidiEvent(GTAR_MIDI_EVENT event) {
	RESULT r = R_OK;

	CBRM_NA((m_gTarPendingMidiEvents_n < MAX_PENDING_EVENTS), "QueueNewMidiEvent: Cannot queue another event as queue is full!");

	 int32_t  i = m_gTarPendingMidiEvents_c;
	 int32_t  j = 0;
	// copy over the event to avoid dynamic allocation
	m_gTarPendingMidiEvents[i].m_gmet = event.m_gmet;
	m_gTarPendingMidiEvents[i].m_params_n = event.m_params_n;
	for(j = 0; j < event.m_params_n; j++)
		m_gTarPendingMidiEvents[i].m_params[j] = event.m_params[j];

	m_gTarPendingMidiEvents_n++;

	// circular buffer style
	m_gTarPendingMidiEvents_c++;
	if(m_gTarPendingMidiEvents_c == MAX_PENDING_EVENTS)
		m_gTarPendingMidiEvents_c = 0;

	DEBUG_LINEOUT("Queued up new event 0x%x c:%d n:%d e:%d", event.m_gmet, m_gTarPendingMidiEvents_c, m_gTarPendingMidiEvents_n, m_gTarPendingMidiEvents_e);

Error:
	return r;
}

uint8_t IsMidiEventPending() {
	return m_gTarPendingMidiEvents_n;
}

// TODO: Pass to Gtar Handle Queue Event
RESULT ExecuteQueuedMidiEvent() {
	RESULT r = R_OK;

	CBRM_NA((m_gTarPendingMidiEvents_n > 0), "ExecuteQueuedMidiEvent: Cannot execute event as queue is empty");

	int32_t i = m_gTarPendingMidiEvents_e;

	DEBUG_LINEOUT("Executing event 0x%x", m_gTarPendingMidiEvents[i].m_gmet);

	switch(m_gTarPendingMidiEvents[i].m_gmet) {
		case GTAR_SEND_MIDI_NOTE: {
			CRM_NA(SendMidiNoteMsg( m_gTarPendingMidiEvents[i].m_params[0],
									m_gTarPendingMidiEvents[i].m_params[1],
									m_gTarPendingMidiEvents[i].m_params[2],
									m_gTarPendingMidiEvents[i].m_params[3]), g_SendUSBMidiNoteMsg_errmsg);
		} break;

		case GTAR_SEND_MIDI_FRET: {
			CRM_NA(SendMidiFret( m_gTarPendingMidiEvents[i].m_params[0],
								 m_gTarPendingMidiEvents[i].m_params[1],
								 m_gTarPendingMidiEvents[i].m_params[2]), g_SendUSBMidiFret_errmsg);
		} break;

		case GTAR_SEND_FW_VERSION: {
			CRM_NA(SendFirmwareVersion(), g_SendFirmwareVersion_errmsg);
		} break;

		case GTAR_SEND_FW_ACK: {
			CRM_NA(SendFirmwareDownloadAck( m_gTarPendingMidiEvents[i].m_params[0]), g_SendFirmwareDownloadAck_errmsg);
		} break;

		case GTAR_SEND_PIEZO_FW_ACK: {
			CRM_NA(SendPiezoFirmwareDownloadAck( m_gTarPendingMidiEvents[i].m_params[0]), g_SendPiezoFirmwareDownloadAck_errmsg);
		} break;

		case GTAR_SEND_BATTERY_STATUS: {
			CRM_NA(SendBatteryStatusAck(), g_SendBatteryStatusAck_errmsg);
		} break;

		case GTAR_SEND_BATTERY_CHARGE: {
			CRM_NA(SendBatteryChargePercentageAck(), g_SendBatteryChargePercentageAck_errmsg);
		} break;

		case GTAR_SEND_SERIAL_NUMBER: {
			CRM_NA(SendRequestSerialNumberAck(m_gTarPendingMidiEvents[i].m_params[0]), g_SendRequestSerialNumberAck_errmsg);
		} break;

		case GTAR_SEND_PIEZO_CT_MATRIX: {
			CRM_NA(SendGetPiezoCrossTalkMatrixAck(m_gTarPendingMidiEvents[i].m_params[0], m_gTarPendingMidiEvents[i].m_params[1]), g_SendGetPiezoCrossTalkMatrixAck_errmsg);
		} break;

		case GTAR_SEND_PIEZO_SENSITIVITY: {
			CRM_NA(SendGetPiezoSensitivityAck(m_gTarPendingMidiEvents[i].m_params[0]), g_SendGetPiezoSensitivityAck_errmsg);
		} break;

		case GTAR_SEND_PIEZO_WINDOW: {
			CRM_NA(SendGetPiezoWindowAck(m_gTarPendingMidiEvents[i].m_params[0]), g_SendGetPiezoWindowAck_errmsg);
		} break;

		case GTAR_SEND_CALIBRATE_PIEZO_STRING: {
			CRM_NA(SendCalibratePiezoStringAck(m_gTarPendingMidiEvents[i].m_params[0]), g_SendCalibratePiezoStringAck_errmsg);
		} break;

		case GTAR_SEND_PIEZO_CMD_ACK: {
			CRM_NA(SendPiezoCmdAck(m_gTarPendingMidiEvents[i].m_params[0]), g_SendPiezoCmdAck_errmsg);
		} break;

		case GTAR_SEND_PIEZO_CMD_RESPONSE: {
			CRM_NA(SendAck((uint8_t *)(m_gTarPendingMidiEvents[i].m_params)), g_SendAck_errmsg);
		} break;

		case GTAR_SEND_COMMIT_USERSPACE: {
			CRM_NA(SendCommitUserspaceAck(m_gTarPendingMidiEvents[i].m_params[0]), g_SendCommitUserspaceAck_errmsg);
		} break;

		case GTAR_SEND_RESET_USERSPACE: {
			CRM_NA(SendResetUserspaceAck(m_gTarPendingMidiEvents[i].m_params[0]), g_SendResetUserspaceAck_errmsg);
		} break;

		default: {
			CRM(0, "Unhandled queued msg type: 0x%x", m_gTarPendingMidiEvents[i].m_gmet);
		} break;
	}

Error:
	m_gTarPendingMidiEvents_n--;

	m_gTarPendingMidiEvents_e++;
		if(m_gTarPendingMidiEvents_e == MAX_PENDING_EVENTS)
			m_gTarPendingMidiEvents_e = 0;

	DEBUG_LINEOUT("Event executed c:%d n:%d e:%d", m_gTarPendingMidiEvents_c, m_gTarPendingMidiEvents_n, m_gTarPendingMidiEvents_e);
	return r;
}
