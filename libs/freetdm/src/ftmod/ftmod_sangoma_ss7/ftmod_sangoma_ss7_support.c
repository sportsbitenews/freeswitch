/*
 * Copyright (c) 2009, Konrad Hammel <konrad@sangoma.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * * Neither the name of the original author; nor the names of any contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* INCLUDE ********************************************************************/
#include "ftmod_sangoma_ss7_main.h"
/******************************************************************************/

/* DEFINES ********************************************************************/
/******************************************************************************/

/* GLOBALS ********************************************************************/
uint32_t sngss7_id;
/******************************************************************************/

/* PROTOTYPES *****************************************************************/
int check_for_state_change(ftdm_channel_t *ftdmchan);
int check_cics_in_range(sngss7_chan_data_t *sngss7_info);
int check_for_reset(sngss7_chan_data_t *sngss7_info);

unsigned long get_unique_id(void);

ftdm_status_t extract_chan_data(uint32_t circuit, sngss7_chan_data_t **sngss7_info, ftdm_channel_t **ftdmchan);

ftdm_status_t check_if_rx_grs_started(ftdm_span_t *ftdmspan);
ftdm_status_t check_if_rx_grs_processed(ftdm_span_t *ftdmspan);
ftdm_status_t check_if_rx_gra_started(ftdm_span_t *ftdmspan);
ftdm_status_t check_for_res_sus_flag(ftdm_span_t *ftdmspan);

ftdm_status_t process_span_ucic(ftdm_span_t *ftdmspan);

ftdm_status_t clear_rx_grs_flags(sngss7_chan_data_t *sngss7_info);
ftdm_status_t clear_tx_grs_flags(sngss7_chan_data_t *sngss7_info);
ftdm_status_t clear_rx_rsc_flags(sngss7_chan_data_t *sngss7_info);
ftdm_status_t clear_tx_rsc_flags(sngss7_chan_data_t *sngss7_info);
ftdm_status_t clear_rx_grs_data(sngss7_chan_data_t *sngss7_info);
ftdm_status_t clear_rx_gra_data(sngss7_chan_data_t *sngss7_info);
ftdm_status_t clear_tx_grs_data(sngss7_chan_data_t *sngss7_info);

ftdm_status_t encode_subAddrIE_nsap(const char *subAddr, char *subAddrIE, int type);
ftdm_status_t encode_subAddrIE_nat(const char *subAddr, char *subAddrIE, int type);

int find_mtp2_error_type_in_map(const char *err_type);
int find_link_type_in_map(const char *linkType);
int find_switch_type_in_map(const char *switchType);
int find_ssf_type_in_map(const char *ssfType);
int find_cic_cntrl_in_map(const char *cntrlType);

ftdm_status_t check_status_of_all_isup_intf(void);
ftdm_status_t check_for_reconfig_flag(ftdm_span_t *ftdmspan);

void sngss7_send_signal(sngss7_chan_data_t *sngss7_info, ftdm_signal_event_t event_id);
void sngss7_set_sig_status(sngss7_chan_data_t *sngss7_info, ftdm_signaling_status_t status);
ftdm_status_t sngss7_add_var(sngss7_chan_data_t *ss7_info, const char* var, const char* val);
ftdm_status_t sngss7_add_raw_data(sngss7_chan_data_t *sngss7_info, uint8_t* data, ftdm_size_t data_len);
/******************************************************************************/

FTDM_ENUM_NAMES(CKT_FLAGS_NAMES, CKT_FLAGS_STRING)
FTDM_STR2ENUM(ftmod_ss7_ckt_state2flag, ftmod_ss7_ckt_flag2str, sng_ckt_flag_t, CKT_FLAGS_NAMES, 31)

FTDM_ENUM_NAMES(BLK_FLAGS_NAMES, BLK_FLAGS_STRING)
FTDM_STR2ENUM(ftmod_ss7_blk_state2flag, ftmod_ss7_blk_flag2str, sng_ckt_block_flag_t, BLK_FLAGS_NAMES, 31)

/* FUNCTIONS ******************************************************************/
ftdm_status_t copy_cgPtyNum_from_sngss7(ftdm_channel_t *ftdmchan, SiCgPtyNum *cgPtyNum)
{

	return FTDM_SUCCESS;
}

ftdm_status_t copy_cgPtyNum_to_sngss7(ftdm_channel_t *ftdmchan, SiCgPtyNum *cgPtyNum)
{
	const char *val;	

	ftdm_caller_data_t *caller_data = &ftdmchan->caller_data;

	cgPtyNum->eh.pres		   = PRSNT_NODEF;
	
	cgPtyNum->natAddrInd.pres   = PRSNT_NODEF;
	cgPtyNum->natAddrInd.val	= 0x03;

	
	cgPtyNum->scrnInd.pres	  = PRSNT_NODEF;
	val = ftdm_usrmsg_get_var(ftdmchan->usrmsg, "ss7_screen_ind");
	if (!ftdm_strlen_zero(val)) {
		cgPtyNum->scrnInd.val	= atoi(val);
	} else {
		cgPtyNum->scrnInd.val	= caller_data->screen;
	}
	ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "Calling Party Number Screening Ind %d\n", cgPtyNum->scrnInd.val);
	
	cgPtyNum->presRest.pres	 = PRSNT_NODEF;
	val = ftdm_usrmsg_get_var(ftdmchan->usrmsg, "ss7_pres_ind");
	if (!ftdm_strlen_zero(val)) {
		cgPtyNum->presRest.val	= atoi(val);
	} else {
		cgPtyNum->presRest.val	= caller_data->pres;
	}
	ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "Calling Party Number Presentation Ind %d\n", cgPtyNum->presRest.val);

	cgPtyNum->numPlan.pres	  = PRSNT_NODEF;
	cgPtyNum->numPlan.val	   = 0x01;

	cgPtyNum->niInd.pres		= PRSNT_NODEF;
	cgPtyNum->niInd.val		 = 0x00;

	return copy_tknStr_to_sngss7(caller_data->cid_num.digits, &cgPtyNum->addrSig, &cgPtyNum->oddEven);
}

ftdm_status_t copy_cdPtyNum_from_sngss7(ftdm_channel_t *ftdmchan, SiCdPtyNum *cdPtyNum)
{
	/* TODO: Implement me */
	return FTDM_SUCCESS;
}


ftdm_status_t copy_cdPtyNum_to_sngss7(ftdm_channel_t *ftdmchan, SiCdPtyNum *cdPtyNum)
{
	const char	*cld_nadi = NULL;
	ftdm_caller_data_t *caller_data = &ftdmchan->caller_data;
	sngss7_chan_data_t	*sngss7_info = ftdmchan->call_data;

	cdPtyNum->eh.pres		   = PRSNT_NODEF;

	cdPtyNum->natAddrInd.pres   = PRSNT_NODEF;
	cld_nadi = ftdm_usrmsg_get_var(ftdmchan->usrmsg, "ss7_cld_nadi");
	if (!ftdm_strlen_zero(cld_nadi)) {
		ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "Found user supplied Called NADI value \"%s\"\n", cld_nadi);
		cdPtyNum->natAddrInd.val	= atoi(cld_nadi);
	} else {
		cdPtyNum->natAddrInd.val	= g_ftdm_sngss7_data.cfg.isupCkt[sngss7_info->circuit->id].cld_nadi;
		ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "No user supplied NADI value found for CLD, using \"%d\"\n", cdPtyNum->natAddrInd.val);
	}

	cdPtyNum->numPlan.pres	  = PRSNT_NODEF;
	cdPtyNum->numPlan.val	   = 0x01;

	cdPtyNum->innInd.pres	   = PRSNT_NODEF;
	cdPtyNum->innInd.val		= 0x01;
	
	return copy_tknStr_to_sngss7(caller_data->dnis.digits, &cdPtyNum->addrSig, &cdPtyNum->oddEven);
}

ftdm_status_t copy_genNmb_to_sngss7(ftdm_channel_t *ftdmchan, SiGenNum *genNmb)
{	
	const char *val = NULL;
	sngss7_chan_data_t	*sngss7_info = ftdmchan->call_data;
	SS7_FUNC_TRACE_ENTER(__FUNCTION__);
	
	val = ftdm_usrmsg_get_var(ftdmchan->usrmsg, "ss7_gn_digits");
	if (!ftdm_strlen_zero(val)) {
		ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "Found user supplied Generic Number qualifier \"%s\"\n", val);
		if (copy_tknStr_to_sngss7((char*)val, &genNmb->addrSig, &genNmb->oddEven) != FTDM_SUCCESS) {
			return FTDM_FAIL;
		}
	} else {
		ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "No user supplied Generic Number qualifier \"%s\"\n", val);
		return FTDM_SUCCESS;
	}
	genNmb->eh.pres = PRSNT_NODEF;
	genNmb->addrSig.pres = PRSNT_NODEF;
	
	genNmb->nmbQual.pres = PRSNT_NODEF;
	val = ftdm_usrmsg_get_var(ftdmchan->usrmsg, "ss7_gn_numqual");
	if (!ftdm_strlen_zero(val)) {
		ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "Found user supplied Generic Number \"%s\"\n", val);
		genNmb->nmbQual.val	= atoi(val);
	} else {
		genNmb->nmbQual.val	= g_ftdm_sngss7_data.cfg.isupCkt[sngss7_info->circuit->id].gn_nmbqual;
		ftdm_log_chan_msg(ftdmchan, FTDM_LOG_DEBUG, "No user supplied Generic Number \n");
	}
	genNmb->natAddrInd.pres = PRSNT_NODEF;
	val = ftdm_usrmsg_get_var(ftdmchan->usrmsg, "ss7_gn_nadi");
	if (!ftdm_strlen_zero(val)) {
		ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "Found user supplied Generic Number \"nature of address\" \"%s\"\n", val);
		genNmb->natAddrInd.val	= atoi(val);
	} else {
		genNmb->natAddrInd.val	= g_ftdm_sngss7_data.cfg.isupCkt[sngss7_info->circuit->id].gn_nadi;
		ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "No user supplied Generic Number \"nature of address\" \"%d\"\n", genNmb->natAddrInd.val);
	}
	genNmb->scrnInd.pres = PRSNT_NODEF;
	val = ftdm_usrmsg_get_var(ftdmchan->usrmsg, "ss7_gn_screen_ind");
	if (!ftdm_strlen_zero(val)) {
		ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "Found user supplied Generic Number \"screening indicator\" \"%s\"\n", val);
		genNmb->scrnInd.val	= atoi(val);
	} else {
		genNmb->natAddrInd.val	= g_ftdm_sngss7_data.cfg.isupCkt[sngss7_info->circuit->id].gn_screen_ind;
		ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "No user supplied Generic Number \"screening indicator\" \"%d\"\n", genNmb->natAddrInd.val);
	}
	genNmb->presRest.pres = PRSNT_NODEF;
	val = ftdm_usrmsg_get_var(ftdmchan->usrmsg, "ss7_gn_pres_ind");
	if (!ftdm_strlen_zero(val)) {
		ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "Found user supplied Generic Number \"presentation indicator\" \"%s\"\n", val);
		genNmb->presRest.val	= atoi(val);
	} else {
		genNmb->presRest.val	= g_ftdm_sngss7_data.cfg.isupCkt[sngss7_info->circuit->id].gn_pres_ind;
		ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "No user supplied Generic Number \"presentation indicator\" \"%d\"\n", genNmb->presRest.val);
	}
	genNmb->numPlan.pres = PRSNT_NODEF;
	val = ftdm_usrmsg_get_var(ftdmchan->usrmsg, "ss7_gn_npi");
	if (!ftdm_strlen_zero(val)) {
		ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "Found user supplied Generic Number \"numbering plan\" \"%s\"\n", val);
		genNmb->numPlan.val	= atoi(val);
	} else {
	genNmb->numPlan.val	= g_ftdm_sngss7_data.cfg.isupCkt[sngss7_info->circuit->id].gn_npi;
		ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "No user supplied Generic Number \"numbering plan\" \"%d\"\n", genNmb->numPlan.val);
	}
	genNmb->niInd.pres = PRSNT_NODEF;
	val = ftdm_usrmsg_get_var(ftdmchan->usrmsg, "ss7_gn_num_inc_ind");
	if (!ftdm_strlen_zero(val)) {
		ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "Found user supplied Generic Number \"number incomplete indicator\" \"%s\"\n", val);
		genNmb->niInd.val	= atoi(val);
	} else {
		genNmb->niInd.val	= g_ftdm_sngss7_data.cfg.isupCkt[sngss7_info->circuit->id].gn_num_inc_ind;
		ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "No user supplied Generic Number \"number incomplete indicator\" \"%d\"\n", genNmb->niInd.val);
	}
	SS7_FUNC_TRACE_EXIT(__FUNCTION__);
	return FTDM_SUCCESS;
}

ftdm_status_t copy_genNmb_from_sngss7(ftdm_channel_t *ftdmchan, SiGenNum *genNmb)
{
	char val[64];
	sngss7_chan_data_t	*sngss7_info = ftdmchan->call_data;
	SS7_FUNC_TRACE_ENTER(__FUNCTION__);

	memset(val, 0, sizeof(val));

	if (genNmb->eh.pres != PRSNT_NODEF || genNmb->addrSig.pres != PRSNT_NODEF) {
		ftdm_log_chan_msg(ftdmchan, FTDM_LOG_DEBUG, "No Generic Number available\n");
		return FTDM_SUCCESS;
	}

	copy_tknStr_from_sngss7(genNmb->addrSig, val, genNmb->oddEven);

	ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "Generic Number:%s\n", val);
	sngss7_add_var(sngss7_info, "ss7_gn_digits", val);

	if (genNmb->nmbQual.pres == PRSNT_NODEF) {
		snprintf(val, sizeof(val), "%d", genNmb->nmbQual.val);
		ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "Generic Number \"number qualifier\" \n", val);
		sngss7_add_var(sngss7_info, "ss7_gn_numqual", val);
	}

	if (genNmb->natAddrInd.pres == PRSNT_NODEF) {
		snprintf(val, sizeof(val), "%d", genNmb->natAddrInd.val);
		ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "Generic Number \"nature of address\" \"%s\"\n", val);
		sngss7_add_var(sngss7_info, "ss7_gn_nadi", val);
	}

	if (genNmb->scrnInd.pres == PRSNT_NODEF) {
		snprintf(val, sizeof(val), "%d", genNmb->scrnInd.val);
		ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "Generic Number \"screening indicator\" \"%s\"\n", val);
		sngss7_add_var(sngss7_info, "ss7_gn_screen_ind", val);
	}
	
	if (genNmb->presRest.pres == PRSNT_NODEF) {
		snprintf(val, sizeof(val), "%d", genNmb->presRest.val);
		ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "Generic Number \"presentation indicator\" \"%s\"\n", val);
		sngss7_add_var(sngss7_info, "ss7_gn_pres_ind", val);
	}

	if (genNmb->numPlan.pres == PRSNT_NODEF) {
		snprintf(val, sizeof(val), "%d", genNmb->numPlan.val);
		ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "Generic Number \"numbering plan\" \"%s\"\n", val);
		sngss7_add_var(sngss7_info, "ss7_gn_npi", val);
	}

	if (genNmb->niInd.pres == PRSNT_NODEF) {
		snprintf(val, sizeof(val), "%d", genNmb->niInd.val);
		ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "Generic Number \"number incomplete indicator\" \"%s\"\n", val);
		sngss7_add_var(sngss7_info, "ss7_gn_num_inc_ind", val);
	}
	
	SS7_FUNC_TRACE_EXIT(__FUNCTION__);
	return FTDM_SUCCESS;
}

ftdm_status_t copy_redirgNum_to_sngss7(ftdm_channel_t *ftdmchan, SiRedirNum *redirgNum)
{
	const char* val = NULL;
	sngss7_chan_data_t	*sngss7_info = ftdmchan->call_data;	
	ftdm_caller_data_t *caller_data = &ftdmchan->caller_data;

	val = ftdm_usrmsg_get_var(ftdmchan->usrmsg, "ss7_rdnis_digits");
	if (!ftdm_strlen_zero(val)) {
		ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "Found user supplied Redirection Number\"%s\"\n", val);
		if (copy_tknStr_to_sngss7((char*)val, &redirgNum->addrSig, &redirgNum->oddEven) != FTDM_SUCCESS) {
			return FTDM_FAIL;
		}
	} else if (!ftdm_strlen_zero(caller_data->rdnis.digits)) {
		ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "Found user supplied Redirection Number\"%s\"\n", val);
		if (copy_tknStr_to_sngss7(caller_data->rdnis.digits, &redirgNum->addrSig, &redirgNum->oddEven) != FTDM_SUCCESS) {
			return FTDM_FAIL;
		}
	} else {
		ftdm_log_chan_msg(ftdmchan, FTDM_LOG_DEBUG, "No user supplied Redirection Number\n");
		return FTDM_SUCCESS;
	}
	
	redirgNum->eh.pres = PRSNT_NODEF;

	/* Nature of address indicator */
	redirgNum->natAddr.pres = PRSNT_NODEF;
	
	val = ftdm_usrmsg_get_var(ftdmchan->usrmsg, "ss7_rdnis_nadi");
	if (!ftdm_strlen_zero(val)) {
		redirgNum->natAddr.val = atoi(val);
	} else {		
		redirgNum->natAddr.val = g_ftdm_sngss7_data.cfg.isupCkt[sngss7_info->circuit->id].rdnis_nadi;
	}
	ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "Redirecting Number NADI:%d\n", redirgNum->natAddr.val);

	/* Screening indicator */
	redirgNum->scrInd.pres = PRSNT_NODEF;
	val = ftdm_usrmsg_get_var(ftdmchan->usrmsg, "ss7_rdnis_screen_ind");
	if (!ftdm_strlen_zero(val)) {
		redirgNum->scrInd.val = atoi(val);
	} else {
		redirgNum->scrInd.val = FTDM_SCREENING_VERIFIED_PASSED;
	}
	ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "Redirecting Number Screening Ind:%d\n", redirgNum->scrInd.val);
	
	/* Address presentation restricted ind */
	redirgNum->presRest.pres = PRSNT_NODEF;
	
	val = ftdm_usrmsg_get_var(ftdmchan->usrmsg, "ss7_rdnis_pres_ind");
	if (!ftdm_strlen_zero(val)) {
		redirgNum->presRest.val = atoi(val);
	} else {
		redirgNum->presRest.val =  FTDM_PRES_ALLOWED;
	}
	ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "Redirecting Number Address Presentation Restricted Ind:%d\n", redirgNum->presRest.val);

	/* Numbering plan */
	redirgNum->numPlan.pres = PRSNT_NODEF;

	val = ftdm_usrmsg_get_var(ftdmchan->usrmsg, "ss7_rdnis_plan");
	if (!ftdm_strlen_zero(val)) {
		redirgNum->numPlan.val = atoi(val);
	} else {
		redirgNum->numPlan.val = caller_data->rdnis.plan; 
	}
	
	ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "Redirecting Number Numbering plan:%d\n", redirgNum->numPlan.val);

	return copy_tknStr_to_sngss7(caller_data->rdnis.digits, &redirgNum->addrSig, &redirgNum->oddEven);
}

ftdm_status_t copy_redirgNum_from_sngss7(ftdm_channel_t *ftdmchan, SiRedirNum *redirgNum)
{
	char val[20];
	sngss7_chan_data_t	*sngss7_info = ftdmchan->call_data;
	ftdm_caller_data_t *caller_data = &ftdmchan->caller_data;

	if (redirgNum->eh.pres != PRSNT_NODEF || redirgNum->addrSig.pres != PRSNT_NODEF) {
		ftdm_log_chan_msg(ftdmchan, FTDM_LOG_DEBUG, "No Redirecting Number available\n");
		return FTDM_SUCCESS;
	}

	copy_tknStr_from_sngss7(redirgNum->addrSig, ftdmchan->caller_data.rdnis.digits, redirgNum->oddEven);

	ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "Redirecting Number:%s\n", ftdmchan->caller_data.rdnis.digits);
	snprintf(val, sizeof(val), "%s", ftdmchan->caller_data.rdnis.digits);
	sngss7_add_var(sngss7_info, "ss7_rdnis_digits", val);
	

	if (redirgNum->natAddr.pres == PRSNT_NODEF) {
		snprintf(val, sizeof(val), "%d", redirgNum->natAddr.val);
		ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "Redirecting Number NADI:%s\n", val);
		sngss7_add_var(sngss7_info, "ss7_rdnis_nadi", val);
		caller_data->rdnis.type = redirgNum->natAddr.val;
	}

	if (redirgNum->scrInd.pres == PRSNT_NODEF) {
		snprintf(val, sizeof(val), "%d", redirgNum->scrInd.val);
		ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "Redirecting Number Screening Ind:%s\n", val);
		sngss7_add_var(sngss7_info, "ss7_rdnis_screen_ind", val);
	}

	if (redirgNum->presRest.pres == PRSNT_NODEF) {
		snprintf(val, sizeof(val), "%d", redirgNum->presRest.val);
		ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "Redirecting Number Presentation Ind:%s\n", val);
		sngss7_add_var(sngss7_info, "ss7_rdnis_pres_ind", val);		
	}

	if (redirgNum->numPlan.pres == PRSNT_NODEF) {
		snprintf(val, sizeof(val), "%d", redirgNum->numPlan.val);
		ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "Redirecting Number Numbering plan:%s\n", val);
		sngss7_add_var(sngss7_info, "ss7_rdnis_plan", val);
		caller_data->rdnis.plan = redirgNum->numPlan.val;
	}

	return FTDM_SUCCESS;
}

ftdm_status_t copy_tknStr_from_sngss7(TknStr str, char *ftdm, TknU8 oddEven)
{
	uint8_t i;
	uint8_t j;

	/* check if the token string is present */
	if (str.pres == 1) {
		j = 0;

		for (i = 0; i < str.len; i++) {
			sprintf(&ftdm[j], "%X", (str.val[i] & 0x0F));
			j++;
			sprintf(&ftdm[j], "%X", ((str.val[i] & 0xF0) >> 4));
			j++;
		}

		/* if the odd flag is up the last digit is a fake "0" */
		if ((oddEven.pres == 1) && (oddEven.val == 1)) {
			ftdm[j-1] = '\0';
		} else {
			ftdm[j] = '\0';
		}

		
	} else {
		SS7_ERROR("Asked to copy tknStr that is not present!\n");
		return FTDM_FAIL;
	}

	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t append_tknStr_from_sngss7(TknStr str, char *ftdm, TknU8 oddEven)
{
	int i = 0;
	int j = 0;

	/* check if the token string is present */
	if (str.pres == 1) {
		/* find the length of the digits so far */
		j = strlen(ftdm);

		/* confirm that we found an acceptable length */
		if ( j > 25 ) {
			SS7_ERROR("string length exceeds maxium value...aborting append!\n");
			return FTDM_FAIL;
		} /* if ( j > 25 ) */

		/* copy in digits */
		for (i = 0; i < str.len; i++) {
			/* convert 4 bit integer to char and copy into lower nibblet*/
			sprintf(&ftdm[j], "%X", (str.val[i] & 0x0F));
			/* move along */
			j++;
			/* convert 4 bit integer to char and copy into upper nibblet */
			sprintf(&ftdm[j], "%X", ((str.val[i] & 0xF0) >> 4));
			/* move along */
			j++;
		} /* for (i = 0; i < str.len; i++) */

		/* if the odd flag is up the last digit is a fake "0" */
		if ((oddEven.pres == 1) && (oddEven.val == 1)) {
			ftdm[j-1] = '\0';
		} else {
			ftdm[j] = '\0';
		} /* if ((oddEven.pres == 1) && (oddEven.val == 1)) */
	} else {
		SS7_ERROR("Asked to copy tknStr that is not present!\n");
		return FTDM_FAIL;
	} /* if (str.pres == 1) */

	return FTDM_SUCCESS;
}


ftdm_status_t copy_tknStr_to_sngss7(char* val, TknStr *tknStr, TknU8 *oddEven)
{
	char tmp[2];
	int k = 0;
	int j = 0;
	uint8_t flag = 0;
	uint8_t odd = 0;

	uint8_t lower = 0x0;
	uint8_t upper = 0x0;

	tknStr->pres = PRSNT_NODEF;
	
	/* atoi will search through memory starting from the pointer it is given until
	* it finds the \0...since tmp is on the stack it will start going through the
	* possibly causing corruption.  Hard code a \0 to prevent this
	*/
	tmp[1] = '\0';

	while (1) {
		/* grab a digit from the ftdm digits */
		tmp[0] = val[k];

		/* check if the digit is a number and that is not null */
		while (!(isxdigit(tmp[0])) && (tmp[0] != '\0')) {
			SS7_INFO("Dropping invalid digit: %c\n", tmp[0]);
			/* move on to the next value */
			k++;
			tmp[0] = val[k];
		} /* while(!(isdigit(tmp))) */

		/* check if tmp is null or a digit */
		if (tmp[0] != '\0') {
			/* push it into the lower nibble */
			lower = strtol(&tmp[0], (char **)NULL, 16);
			/* move to the next digit */
			k++;
			/* grab a digit from the ftdm digits */
			tmp[0] = val[k];

			/* check if the digit is a number and that is not null */
			while (!(isxdigit(tmp[0])) && (tmp[0] != '\0')) {
				SS7_INFO("Dropping invalid digit: %c\n", tmp[0]);
				k++;
				tmp[0] = val[k];
			} /* while(!(isdigit(tmp))) */

			/* check if tmp is null or a digit */
			if (tmp[0] != '\0') {
				/* push the digit into the upper nibble */
				upper = (strtol(&tmp[0], (char **)NULL, 16)) << 4;
			} else {
				/* there is no upper ... fill in 0 */
				upper = 0x0;
				/* throw the odd flag */
				odd = 1;
				/* throw the end flag */
				flag = 1;
			} /* if (tmp != '\0') */
		} else {
			/* keep the odd flag down */
			odd = 0;
			/* break right away since we don't need to write the digits */
			break;
		}

		/* push the digits into the trillium structure */
		tknStr->val[j] = upper | lower;

		/* increment the trillium pointer */
		j++;

		/* if the flag is up we're through all the digits */
		if (flag) break;

		/* move to the next digit */
		k++;
	} /* while(1) */
	
	tknStr->len = j;
	oddEven->pres = PRSNT_NODEF;
	oddEven->val = odd;
	return FTDM_SUCCESS;
}



/******************************************************************************/
int check_for_state_change(ftdm_channel_t *ftdmchan)
{

	/* check to see if there are any pending state changes on the channel and give them a sec to happen*/
	ftdm_wait_for_flag_cleared(ftdmchan, FTDM_CHANNEL_STATE_CHANGE, 500);

	/* check the flag to confirm it is clear now */

	if (ftdm_test_flag(ftdmchan, FTDM_CHANNEL_STATE_CHANGE)) {
		/* the flag is still up...so we have a problem */
		ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "FTDM_CHANNEL_STATE_CHANGE flag set for over 500ms, channel state = %s\n",
									ftdm_channel_state2str (ftdmchan->state));

		return 1;
	}

	return 0;
}

/******************************************************************************/
int check_cics_in_range(sngss7_chan_data_t *sngss7_info)
{


#if 0
	ftdm_channel_t		*tmp_ftdmchan;
	sngss7_chan_data_t  *tmp_sngss7_info;
	int 				i = 0;
	
	/* check all the circuits in the range to see if we are the last ckt to reset */
	for ( i = sngss7_info->grs.circuit; i < ( sngss7_info->grs.range + 1 ); i++ ) {
		if ( g_ftdm_sngss7_data.cfg.isupCircuit[i].siglink == 0 ) {
		
			/* get the ftdmchan and ss7_chan_data from the circuit */
			if (extract_chan_data(g_ftdm_sngss7_data.cfg.isupCircuit[i].id, &tmp_sngss7_info, &tmp_ftdmchan)) {
				SS7_ERROR("Failed to extract channel data for circuit = %d!\n", g_ftdm_sngss7_data.cfg.isupCircuit[i].id);
				return 0;
			}

			/* check if the channel still has the reset flag done is up */
			if (!sngss7_test_ckt_flag(tmp_sngss7_info, FLAG_GRP_RESET_RX_DN)) {
				SS7_DEBUG_CHAN(tmp_ftdmchan, "[CIC:%d] Still processing reset...\n", tmp_sngss7_info->circuit->cic);
				return 0;
			}
		} /* if not siglink */
	} /* for */

	SS7_DEBUG("All circuits out of reset: circuit=%d, range=%d\n",
				sngss7_info->grs.circuit,
				sngss7_info->grs.range);
	return 1;

#endif

	return 0;

}

/******************************************************************************/
ftdm_status_t extract_chan_data(uint32_t circuit, sngss7_chan_data_t **sngss7_info, ftdm_channel_t **ftdmchan)
{
	/*SS7_FUNC_TRACE_ENTER(__FUNCTION__);*/

	if (g_ftdm_sngss7_data.cfg.isupCkt[circuit].obj == NULL) {
		SS7_ERROR("sngss7_info is Null for circuit #%d\n", circuit);
		return FTDM_FAIL;
	}

	ftdm_assert_return(g_ftdm_sngss7_data.cfg.isupCkt[circuit].obj, FTDM_FAIL, "received message on signalling link or non-configured cic\n");

	*sngss7_info = g_ftdm_sngss7_data.cfg.isupCkt[circuit].obj;

	ftdm_assert_return((*sngss7_info)->ftdmchan, FTDM_FAIL, "received message on signalling link or non-configured cic\n");
	*ftdmchan = (*sngss7_info)->ftdmchan;

	/*SS7_FUNC_TRACE_EXIT(__FUNCTION__);*/
	return FTDM_SUCCESS;
}

/******************************************************************************/
int check_for_reset(sngss7_chan_data_t *sngss7_info)
{

	if (sngss7_test_ckt_flag(sngss7_info,FLAG_RESET_RX)) {
		return 1;
	}
	
	if (sngss7_test_ckt_flag(sngss7_info,FLAG_RESET_TX)) {
		return 1;
	}
	
	if (sngss7_test_ckt_flag(sngss7_info,FLAG_GRP_RESET_RX)) {
		return 1;
	}
	
	if (sngss7_test_ckt_flag(sngss7_info,FLAG_GRP_RESET_TX)) {
		return 1;
	}

	return 0;
	
}

/******************************************************************************/
unsigned long get_unique_id(void)
{
	int	procId = sng_get_procId(); 

	/* id values are between (procId * 1,000,000) and ((procId + 1) * 1,000,000) */ 
	if (sngss7_id < ((procId + 1) * 1000000) ) {
		sngss7_id++;
	} else {
		sngss7_id = procId * 1000000;
	}

	return(sngss7_id);
}

/******************************************************************************/
ftdm_status_t check_if_rx_grs_started(ftdm_span_t *ftdmspan)
{
	ftdm_channel_t 		*ftdmchan = NULL;
	sngss7_chan_data_t  *sngss7_info = NULL;
	sngss7_span_data_t	*sngss7_span = (sngss7_span_data_t *)ftdmspan->signal_data;
	int 				i;


	SS7_INFO("Rx GRS (%d:%d)\n", 
				g_ftdm_sngss7_data.cfg.isupCkt[sngss7_span->rx_grs.circuit].cic, 
				(g_ftdm_sngss7_data.cfg.isupCkt[sngss7_span->rx_grs.circuit].cic + sngss7_span->rx_grs.range));

	for ( i = sngss7_span->rx_grs.circuit; i < (sngss7_span->rx_grs.circuit + sngss7_span->rx_grs.range + 1); i++) {

		/* confirm this is a voice channel, otherwise we do nothing */ 
		if (g_ftdm_sngss7_data.cfg.isupCkt[i].type != VOICE) {
			continue;
		} 

		/* extract the channel in question */
		if (extract_chan_data(i, &sngss7_info, &ftdmchan)) {
			SS7_ERROR("Failed to extract channel data for circuit = %d!\n", i);
			continue;
		}

		/* check if the GRP_RESET_RX flag is already up */
		if (sngss7_test_ckt_flag(sngss7_info, FLAG_GRP_RESET_RX)) {
			/* we have already processed this channel...move along */
			continue;
		}

		/* lock the channel */
		ftdm_mutex_lock(ftdmchan->mutex);

		/* clear up any pending state changes */
		while (ftdm_test_flag (ftdmchan, FTDM_CHANNEL_STATE_CHANGE)) {
			ftdm_sangoma_ss7_process_state_change (ftdmchan);
		}

		/* flag the channel as having received a reset */
		sngss7_set_ckt_flag(sngss7_info, FLAG_GRP_RESET_RX);

		switch (ftdmchan->state) {
		/**************************************************************************/
		case FTDM_CHANNEL_STATE_RESTART:

			/* go to idle so that we can redo the restart state*/
			ftdm_set_state(ftdmchan, FTDM_CHANNEL_STATE_IDLE);

			break;
		/**************************************************************************/
		default:

			/* set the state of the channel to restart...the rest is done by the chan monitor */
			ftdm_set_state(ftdmchan, FTDM_CHANNEL_STATE_RESTART);
			break;
		/**************************************************************************/
		} /* switch (ftdmchan->state) */

		/* unlock the channel again before we exit */
		ftdm_mutex_unlock(ftdmchan->mutex);

	} /* for (chans in GRS */

	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t check_if_rx_grs_processed(ftdm_span_t *ftdmspan)
{
	ftdm_channel_t 		*ftdmchan = NULL;
	sngss7_chan_data_t  *sngss7_info = NULL;
	sngss7_span_data_t	*sngss7_span = (sngss7_span_data_t *)ftdmspan->signal_data;
	int 				i;
	int					byte = 0;
	int					bit = 0;

	/* check all the circuits in the range to see if they are done resetting */
	for ( i = sngss7_span->rx_grs.circuit; i < (sngss7_span->rx_grs.circuit + sngss7_span->rx_grs.range + 1); i++) {

		/* confirm this is a voice channel, otherwise we do nothing */ 
		if (g_ftdm_sngss7_data.cfg.isupCkt[i].type != VOICE) {
			continue;
		}

		/* extract the channel in question */
		if (extract_chan_data(i, &sngss7_info, &ftdmchan)) {
			SS7_ERROR("Failed to extract channel data for circuit = %d!\n", i);
			continue;
		}

		/* lock the channel */
		ftdm_mutex_lock(ftdmchan->mutex);

		/* check if there is a state change pending on the channel */
		if (!ftdm_test_flag(ftdmchan, FTDM_CHANNEL_STATE_CHANGE)) {
			/* check the state to the GRP_RESET_RX_DN flag */
			if (!sngss7_test_ckt_flag(sngss7_info, FLAG_GRP_RESET_RX_DN)) {
				/* this channel is still resetting...do nothing */
					goto GRS_UNLOCK_ALL;
			} /* if (!sngss7_test_ckt_flag(sngss7_info, FLAG_GRP_RESET_RX_DN)) */
		} else {
			/* state change pending */
			goto GRS_UNLOCK_ALL;
		}
	} /* for ( i = circuit; i < (circuit + range + 1); i++) */

	SS7_DEBUG("All circuits out of reset for GRS: circuit=%d, range=%d\n",
					sngss7_span->rx_grs.circuit,
					sngss7_span->rx_grs.range);

	/* check all the circuits in the range to see if they are done resetting */
	for ( i = sngss7_span->rx_grs.circuit; i < (sngss7_span->rx_grs.circuit + sngss7_span->rx_grs.range + 1); i++) {

		/* confirm this is a voice channel, otherwise we do nothing */ 
		if (g_ftdm_sngss7_data.cfg.isupCkt[i].type != VOICE) {
			continue;
		}

		/* extract the channel in question */
		if (extract_chan_data(i, &sngss7_info, &ftdmchan)) {
			SS7_ERROR("Failed to extract channel data for circuit = %d!\n",i);
			/* check if we need to die */
			SS7_ASSERT;
			/* move along */
			continue;
		}

		/* throw the GRP reset flag complete flag */
		sngss7_set_ckt_flag(sngss7_info, FLAG_GRP_RESET_RX_CMPLT);

		/* move the channel to the down state */
		ftdm_set_state(ftdmchan, FTDM_CHANNEL_STATE_DOWN);

		/* update the status map if the ckt is in blocked state */
		if ((sngss7_test_ckt_blk_flag(sngss7_info, FLAG_CKT_MN_BLOCK_RX)) ||
			(sngss7_test_ckt_blk_flag(sngss7_info, FLAG_CKT_MN_BLOCK_TX)) ||
			(sngss7_test_ckt_blk_flag(sngss7_info, FLAG_GRP_MN_BLOCK_RX)) ||
			(sngss7_test_ckt_blk_flag(sngss7_info, FLAG_GRP_MN_BLOCK_RX))) {
		
			sngss7_span->rx_grs.status[byte] = (sngss7_span->rx_grs.status[byte] | (1 << bit));
		} /* if blocked */
		
		/* update the bit and byte counter*/
		bit ++;
		if (bit == 8) {
			byte++;
			bit = 0;
		}
	} /* for ( i = circuit; i < (circuit + range + 1); i++) */

GRS_UNLOCK_ALL:
	for ( i = sngss7_span->rx_grs.circuit; i < (sngss7_span->rx_grs.circuit + sngss7_span->rx_grs.range + 1); i++) {

		/* confirm this is a voice channel, otherwise we do nothing */ 
		if (g_ftdm_sngss7_data.cfg.isupCkt[i].type != VOICE) {
			continue;
		}

		/* extract the channel in question */
		if (extract_chan_data(i, &sngss7_info, &ftdmchan)) {
			SS7_ERROR("Failed to extract channel data for circuit = %d!\n", i);
			continue;
		}

		/* unlock the channel */
		ftdm_mutex_unlock(ftdmchan->mutex);
	}

	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t check_if_rx_gra_started(ftdm_span_t *ftdmspan)
{
	ftdm_channel_t 		*ftdmchan = NULL;
	sngss7_chan_data_t  *sngss7_info = NULL;
	sngss7_span_data_t	*sngss7_span = (sngss7_span_data_t *)ftdmspan->signal_data;
	int 				i;

	SS7_INFO("Rx GRA (%d:%d)\n", 
				g_ftdm_sngss7_data.cfg.isupCkt[sngss7_span->rx_gra.circuit].cic, 
				(g_ftdm_sngss7_data.cfg.isupCkt[sngss7_span->rx_gra.circuit].cic + sngss7_span->rx_gra.range));

	for (i = sngss7_span->rx_gra.circuit; i < (sngss7_span->rx_gra.circuit + sngss7_span->rx_gra.range + 1); i++) {

		/* extract the channel in question */
		if (extract_chan_data(i, &sngss7_info, &ftdmchan)) {
			SS7_ERROR("Failed to extract channel data for circuit = %d!\n", i);
			continue;
		}

		/* check if the channel is already procoessing the GRA */
		if (sngss7_test_ckt_flag(sngss7_info, FLAG_GRP_RESET_TX_RSP)) {
			/* move along */
			continue;
		}

		/* lock the channel */
		ftdm_mutex_lock(ftdmchan->mutex);

		/* clear up any pending state changes */
		while (ftdm_test_flag (ftdmchan, FTDM_CHANNEL_STATE_CHANGE)) {
			ftdm_sangoma_ss7_process_state_change (ftdmchan);
		}



		switch (ftdmchan->state) {
		/**********************************************************************/
		case FTDM_CHANNEL_STATE_RESTART:
			
			/* throw the FLAG_RESET_TX_RSP to indicate we have acknowledgement from the remote side */
			sngss7_set_ckt_flag(sngss7_info, FLAG_GRP_RESET_TX_RSP);

			/* go to DOWN */
			ftdm_set_state(ftdmchan, FTDM_CHANNEL_STATE_DOWN);

			break;
		/**********************************************************************/
		case FTDM_CHANNEL_STATE_DOWN:

			/* do nothing, just drop the message */
			SS7_DEBUG("Receveived GRA in down state, dropping\n");

			break;
		/**********************************************************************/
		case FTDM_CHANNEL_STATE_TERMINATING:
		case FTDM_CHANNEL_STATE_HANGUP:
		case FTDM_CHANNEL_STATE_HANGUP_COMPLETE:
			
			/* throw the FLAG_RESET_TX_RSP to indicate we have acknowledgement from the remote side */
			sngss7_set_ckt_flag(sngss7_info, FLAG_GRP_RESET_TX_RSP);

			break;
		/**********************************************************************/
		default:
			/* ITU Q764-2.9.5.1.c -> release the circuit */
			if (sngss7_span->rx_gra.cause != 0) {
				ftdmchan->caller_data.hangup_cause = sngss7_span->rx_gra.cause;
			} else {
				ftdmchan->caller_data.hangup_cause = 98;	/* Message not compatiable with call state */
			}

			/* go to terminating to hang up the call */
			ftdm_set_state(ftdmchan, FTDM_CHANNEL_STATE_TERMINATING);
			break;
		/**********************************************************************/
		}

		/* unlock the channel again before we exit */
		ftdm_mutex_unlock(ftdmchan->mutex);

	} /* for ( circuits in request */


	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t check_for_res_sus_flag(ftdm_span_t *ftdmspan)
{
	ftdm_channel_t		*ftdmchan = NULL;
	sngss7_chan_data_t	*sngss7_info = NULL;
	ftdm_sigmsg_t 		sigev;
	int 				x;

	for (x = 1; x < (ftdmspan->chan_count + 1); x++) {

		/* extract the channel structure and sngss7 channel data */
		ftdmchan = ftdmspan->channels[x];
		
		/* if the call data is NULL move on */
		if (ftdmchan->call_data == NULL) continue;

		sngss7_info = ftdmchan->call_data;

		/* lock the channel */
		ftdm_mutex_lock(ftdmchan->mutex);

		memset (&sigev, 0, sizeof (sigev));

		sigev.chan_id = ftdmchan->chan_id;
		sigev.span_id = ftdmchan->span_id;
		sigev.channel = ftdmchan;

		/* if we have the PAUSED flag and the sig status is still UP */
		if ((sngss7_test_ckt_flag(sngss7_info, FLAG_INFID_PAUSED)) &&
			(ftdm_test_flag(ftdmchan, FTDM_CHANNEL_SIG_UP))) {

			/* clear up any pending state changes */
			while (ftdm_test_flag (ftdmchan, FTDM_CHANNEL_STATE_CHANGE)) {
				ftdm_sangoma_ss7_process_state_change (ftdmchan);
			}
			
			/* throw the channel into SUSPENDED to process the flag */
			/* after doing this once the sig status will be down */
			ftdm_set_state (ftdmchan, FTDM_CHANNEL_STATE_SUSPENDED);
		}

		/* if the RESUME flag is up go to SUSPENDED to process the flag */
		/* after doing this the flag will be cleared */
		if (sngss7_test_ckt_flag(sngss7_info, FLAG_INFID_RESUME)) {

			/* clear up any pending state changes */
			while (ftdm_test_flag (ftdmchan, FTDM_CHANNEL_STATE_CHANGE)) {
				ftdm_sangoma_ss7_process_state_change (ftdmchan);
			}

			/* got SUSPENDED state to clear the flag */
			ftdm_set_state (ftdmchan, FTDM_CHANNEL_STATE_SUSPENDED);
		}

		/* unlock the channel */
		ftdm_mutex_unlock(ftdmchan->mutex);

	} /* for (x = 1; x < (span->chan_count + 1); x++) */

	/* signal the core that sig events are queued for processing */
	ftdm_span_trigger_signals(ftdmspan);

	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t process_span_ucic(ftdm_span_t *ftdmspan)
{
	ftdm_channel_t 		*ftdmchan = NULL;
	sngss7_chan_data_t  *sngss7_info = NULL;
	sngss7_span_data_t	*sngss7_span = (sngss7_span_data_t *)ftdmspan->signal_data;
	int 				i;

	for (i = sngss7_span->ucic.circuit; i < (sngss7_span->ucic.circuit + sngss7_span->ucic.range + 1); i++) {

		/* extract the channel in question */
		if (extract_chan_data(i, &sngss7_info, &ftdmchan)) {
			SS7_ERROR("Failed to extract channel data for circuit = %d!\n", i);
			continue;
		}

		/* lock the channel */
		ftdm_mutex_lock(ftdmchan->mutex);

		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx UCIC\n", sngss7_info->circuit->cic);

		/* clear up any pending state changes */
		while (ftdm_test_flag (ftdmchan, FTDM_CHANNEL_STATE_CHANGE)) {
			ftdm_sangoma_ss7_process_state_change (ftdmchan);
		}

		/* throw the ckt block flag */
		sngss7_set_ckt_blk_flag(sngss7_info, FLAG_CKT_UCIC_BLOCK);

		/* set the channel to suspended state */
		ftdm_set_state(ftdmchan, FTDM_CHANNEL_STATE_SUSPENDED);

		/* unlock the channel again before we exit */
		ftdm_mutex_unlock(ftdmchan->mutex);
	}

	/* clear out the ucic data since we're done with it */
	memset(&sngss7_span->ucic, 0x0, sizeof(sngss7_group_data_t));

	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t clear_rx_grs_flags(sngss7_chan_data_t *sngss7_info)
{
	/* clear all the flags related to an incoming GRS */
	sngss7_clear_ckt_flag(sngss7_info, FLAG_GRP_RESET_RX);
	sngss7_clear_ckt_flag(sngss7_info, FLAG_GRP_RESET_RX_DN);
	sngss7_clear_ckt_flag(sngss7_info, FLAG_GRP_RESET_RX_CMPLT);

	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t clear_rx_grs_data(sngss7_chan_data_t *sngss7_info)
{
	ftdm_channel_t		*ftdmchan = sngss7_info->ftdmchan;
	sngss7_span_data_t	*sngss7_span = ftdmchan->span->signal_data;

	/* clear the rx_grs data fields */
	memset(&sngss7_span->rx_grs, 0x0, sizeof(sngss7_group_data_t));

	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t clear_rx_gra_data(sngss7_chan_data_t *sngss7_info)
{
	ftdm_channel_t		*ftdmchan = sngss7_info->ftdmchan;
	sngss7_span_data_t	*sngss7_span = ftdmchan->span->signal_data;

	/* clear the rx_grs data fields */
	memset(&sngss7_span->rx_gra, 0x0, sizeof(sngss7_group_data_t));

	return FTDM_SUCCESS;
}
/******************************************************************************/
ftdm_status_t clear_tx_grs_flags(sngss7_chan_data_t *sngss7_info)
{
	/* clear all the flags related to an outgoing GRS */
	sngss7_clear_ckt_flag(sngss7_info, FLAG_GRP_RESET_BASE);
	sngss7_clear_ckt_flag(sngss7_info, FLAG_GRP_RESET_TX);
	sngss7_clear_ckt_flag(sngss7_info, FLAG_GRP_RESET_SENT);
	sngss7_clear_ckt_flag(sngss7_info, FLAG_GRP_RESET_TX_RSP);

	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t clear_tx_grs_data(sngss7_chan_data_t *sngss7_info)
{
	ftdm_channel_t		*ftdmchan = sngss7_info->ftdmchan;
	sngss7_span_data_t	*sngss7_span = ftdmchan->span->signal_data;

	/* clear the rx_grs data fields */
	memset(&sngss7_span->tx_grs, 0x0, sizeof(sngss7_group_data_t));

	return FTDM_SUCCESS;
}

/******************************************************************************/

/******************************************************************************/
ftdm_status_t clear_rx_rsc_flags(sngss7_chan_data_t *sngss7_info)
{
	/* clear all the flags related to an incoming RSC */
	sngss7_clear_ckt_flag(sngss7_info, FLAG_RESET_RX);

	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t clear_tx_rsc_flags(sngss7_chan_data_t *sngss7_info)
{
	/* clear all the flags related to an outgoing RSC */
	sngss7_clear_ckt_flag(sngss7_info, FLAG_RESET_TX);
	sngss7_clear_ckt_flag(sngss7_info, FLAG_RESET_SENT);
	sngss7_clear_ckt_flag(sngss7_info, FLAG_RESET_TX_RSP);

	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t encode_subAddrIE_nsap(const char *subAddr, char *subAddrIE, int type)
{
	/* Q931 4.5.9 
	 * 8	7	6	5	4	3	2	1	(octet)
	 *
	 * 0	1	1	1	0	0	0	1	(spare 8) ( IE id 1-7)
	 * X	X	X	X	X	X	X	X	(length of IE contents)
	 * 1	0	0	0	Z	0	0	0	(ext 8) (NSAP type 5-7) (odd/even 4) (spare 1-3)
	 * X	X	X	X	X	X	X	X	(sub address encoded in ia5)
	 */

	int	x = 0;
	int p = 0;
	int len = 0;
	char tmp[2];

	/* initalize the second element of tmp to \0 so that atoi doesn't go to far */
	tmp[1]='\0';

	/* set octet 1 aka IE id */
	p = 0;
	switch(type) {
	/**************************************************************************/
	case SNG_CALLED:						/* called party sub address */
		subAddrIE[p] = 0x71;
		break;
	/**************************************************************************/
	case SNG_CALLING:						/* calling party sub address */
		subAddrIE[p] = 0x6d;
		break;
	/**************************************************************************/
	default:								/* not good */
		SS7_ERROR("Sub-Address type is invalid: %d\n", type);
		return FTDM_FAIL;
		break;
	/**************************************************************************/
	} /* switch(type) */

	/* set octet 3 aka type and o/e */
	p = 2;
	subAddrIE[p] = 0x80;

	/* set the subAddrIE pointer octet 4 */
	p = 3;

	/* loop through all digits in subAddr and insert them into subAddrIE */
	while (subAddr[x] != '\0') {

		/* grab a character */
		tmp[0] = subAddr[x];

		/* confirm it is a digit */
		if (!isdigit(tmp[0])) {
			SS7_INFO("Dropping invalid digit: %c\n", tmp[0]);
			/* move to the next character in subAddr */
			x++;

			/* restart the loop */
			continue;
		}

		/* convert the character to IA5 encoding and write into subAddrIE */
		subAddrIE[p] = atoi(&tmp[0]);	/* lower nibble is the digit */
		subAddrIE[p] |= 0x3 << 4;		/* upper nibble is 0x3 */

		/* increment address length counter */
		len++;

		/* increment the subAddrIE pointer */
		p++;

		/* move to the next character in subAddr */
		x++;

	} /* while (subAddr[x] != '\0') */

	/* set octet 2 aka length of subaddr */
	p = 1;
	subAddrIE[p] = len + 1;


	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t encode_subAddrIE_nat(const char *subAddr, char *subAddrIE, int type)
{
	/* Q931 4.5.9 
	 * 8	7	6	5	4	3	2	1	(octet)
	 *
	 * 0	1	1	1	0	0	0	1	(spare 8) ( IE id 1-7)
	 * X	X	X	X	X	X	X	X	(length of IE contents)
	 * 1	0	0	0	Z	0	0	0	(ext 8) (NSAP type 5-7) (odd/even 4) (spare 1-3)
	 * X	X	X	X	X	X	X	X	(sub address encoded in ia5)
	 */

	int		x = 0;
	int 	p = 0;
	int 	len = 0;
	char 	tmp[2];
	int 	flag = 0;
	int 	odd = 0;
	uint8_t	lower = 0x0;
	uint8_t upper = 0x0;

	/* initalize the second element of tmp to \0 so that atoi doesn't go to far */
	tmp[1]='\0';

	/* set octet 1 aka IE id */
	p = 0;
	switch(type) {
	/**************************************************************************/
	case SNG_CALLED:						/* called party sub address */
		subAddrIE[p] = 0x71;
		break;
	/**************************************************************************/
	case SNG_CALLING:						/* calling party sub address */
		subAddrIE[p] = 0x6d;
		break;
	/**************************************************************************/
	default:								/* not good */
		SS7_ERROR("Sub-Address type is invalid: %d\n", type);
		return FTDM_FAIL;
		break;
	/**************************************************************************/
	} /* switch(type) */

	/* set the subAddrIE pointer octet 4 */
	p = 3;

	/* loop through all digits in subAddr and insert them into subAddrIE */
	while (1) {

		/* grab a character */
		tmp[0] = subAddr[x];

		/* confirm it is a hex digit */
		while ((!isxdigit(tmp[0])) && (tmp[0] != '\0')) {
			SS7_INFO("Dropping invalid digit: %c\n", tmp[0]);
			/* move to the next character in subAddr */
			x++;
			tmp[0] = subAddr[x];
		}

		/* check if tmp is null or a digit */
		if (tmp[0] != '\0') {
			/* push it into the lower nibble using strtol to allow a-f chars */
			lower = strtol(&tmp[0], (char **)NULL, 16);
			/* move to the next digit */
			x++;
			/* grab a digit from the ftdm digits */
			tmp[0] = subAddr[x];

			/* check if the digit is a hex digit and that is not null */
			while (!(isxdigit(tmp[0])) && (tmp[0] != '\0')) {
				SS7_INFO("Dropping invalid digit: %c\n", tmp[0]);
				x++;
				tmp[0] = subAddr[x];
			} /* while(!(isdigit(tmp))) */

			/* check if tmp is null or a digit */
			if (tmp[0] != '\0') {
				/* push the digit into the upper nibble using strtol to allow a-f chars */
				upper = (strtol(&tmp[0], (char **)NULL, 16)) << 4;
			} else {
				/* there is no upper ... fill in spare */
				upper = 0x00;
				/* throw the odd flag since we need to buffer */
				odd = 1;
				/* throw the end flag */
				flag = 1;
			} /* if (tmp != '\0') */
		} else {
			/* keep the odd flag down */
			odd = 0;

			/* throw the flag */
			flag = 1;

			/* bounce out right away */
			break;
		}

		/* fill in the octet */
		subAddrIE[p] = upper | lower;

		/* increment address length counter */
		len++;

		/* if the flag is we're through all the digits */
		if (flag) break;

		/* increment the subAddrIE pointer */
		p++;

		/* move to the next character in subAddr */
		x++;

	} /* while (subAddr[x] != '\0') */

	/* set octet 2 aka length of subaddr */
	p = 1;
	subAddrIE[p] = len + 1;

	/* set octet 3 aka type and o/e */
	p = 2;
	subAddrIE[p] = 0xa0 | (odd << 3);


	return FTDM_SUCCESS;
}

/******************************************************************************/
int find_mtp2_error_type_in_map(const char *err_type)
{
	int i = 0;

	while (sng_mtp2_error_type_map[i].init == 1) {
		/* check if string matches the sng_type name */ 
		if (!strcasecmp(err_type, sng_mtp2_error_type_map[i].sng_type)) {
			/* we've found a match break from the loop */
			break;
		} else {
			/* move on to the next on */
			i++;
		}
	} /* while (sng_mtp2_error_type_map[i].init == 1) */

	/* check how we exited the loop */
	if (sng_mtp2_error_type_map[i].init == 0) {
		return -1;
	} else {
		return i;
	} /* if (sng_mtp2_error_type_map[i].init == 0) */
}

/******************************************************************************/
int find_link_type_in_map(const char *linkType)
{
	int i = 0;

	while (sng_link_type_map[i].init == 1) {
		/* check if string matches the sng_type name */ 
		if (!strcasecmp(linkType, sng_link_type_map[i].sng_type)) {
			/* we've found a match break from the loop */
			break;
		} else {
			/* move on to the next on */
			i++;
		}
	} /* while (sng_link_type_map[i].init == 1) */

	/* check how we exited the loop */
	if (sng_link_type_map[i].init == 0) {
		return -1;
	} else {
		return i;
	} /* if (sng_link_type_map[i].init == 0) */
}

/******************************************************************************/
int find_switch_type_in_map(const char *switchType)
{
	int i = 0;

	while (sng_switch_type_map[i].init == 1) {
		/* check if string matches the sng_type name */ 
		if (!strcasecmp(switchType, sng_switch_type_map[i].sng_type)) {
			/* we've found a match break from the loop */
			break;
		} else {
			/* move on to the next on */
			i++;
		}
	} /* while (sng_switch_type_map[i].init == 1) */

	/* check how we exited the loop */
	if (sng_switch_type_map[i].init == 0) {
		return -1;
	} else {
		return i;
	} /* if (sng_switch_type_map[i].init == 0) */
}

/******************************************************************************/
int find_ssf_type_in_map(const char *ssfType)
{
	int i = 0;

	while (sng_ssf_type_map[i].init == 1) {
		/* check if string matches the sng_type name */ 
		if (!strcasecmp(ssfType, sng_ssf_type_map[i].sng_type)) {
			/* we've found a match break from the loop */
			break;
		} else {
			/* move on to the next on */
			i++;
		}
	} /* while (sng_ssf_type_map[i].init == 1) */

	/* check how we exited the loop */
	if (sng_ssf_type_map[i].init == 0) {
		return -1;
	} else {
		return i;
	} /* if (sng_ssf_type_map[i].init == 0) */
}

/******************************************************************************/
int find_cic_cntrl_in_map(const char *cntrlType)
{
	int i = 0;

	while (sng_cic_cntrl_type_map[i].init == 1) {
		/* check if string matches the sng_type name */ 
		if (!strcasecmp(cntrlType, sng_cic_cntrl_type_map[i].sng_type)) {
			/* we've found a match break from the loop */
			break;
		} else {
			/* move on to the next on */
			i++;
		}
	} /* while (sng_cic_cntrl_type_map[i].init == 1) */

	/* check how we exited the loop */
	if (sng_cic_cntrl_type_map[i].init == 0) {
		return -1;
	} else {
		return i;
	} /* if (sng_cic_cntrl_type_map[i].init == 0) */
}

/******************************************************************************/
ftdm_status_t check_status_of_all_isup_intf(void)
{
	sng_isup_inf_t		*sngss7_intf = NULL;
	uint8_t				status = 0xff;
	int					x;

	/* go through all the isupIntfs and ask the stack to give their current state */
	x = 1;
	for (x = 1; x < (MAX_ISUP_INFS); x++) {
	/**************************************************************************/

		if (g_ftdm_sngss7_data.cfg.isupIntf[x].id == 0) continue;

		sngss7_intf = &g_ftdm_sngss7_data.cfg.isupIntf[x];

		if (ftmod_ss7_isup_intf_sta(sngss7_intf->id, &status)) {
			SS7_ERROR("Failed to get status of ISUP intf %d\n", sngss7_intf->id);
			continue;
		}

		switch (status){
		/**********************************************************************/
		case (SI_INTF_AVAIL):
			SS7_DEBUG("State of ISUP intf %d = AVAIL\n", sngss7_intf->id); 

			/* check the current state for interface that we know */
			if (sngss7_test_flag(sngss7_intf, SNGSS7_PAUSED)) {
				/* we thing the intf is paused...put into resume */ 
				sngss7_clear_flag(sngss7_intf, SNGSS7_PAUSED);
			} else {
				/* nothing to since we already know that interface is active */
			}
			break;
		/**********************************************************************/
		case (SI_INTF_UNAVAIL):
			SS7_DEBUG("State of ISUP intf %d = UNAVAIL\n", sngss7_intf->id); 
			/* check the current state for interface that we know */
			if (sngss7_test_flag(sngss7_intf, SNGSS7_PAUSED)) {
				/* nothing to since we already know that interface is active */ 
			} else {
				/* put the interface into pause */
				sngss7_set_flag(sngss7_intf, SNGSS7_PAUSED);
			}
			break;
		/**********************************************************************/
		case (SI_INTF_CONG1):
			SS7_DEBUG("State of ISUP intf %d = Congestion 1\n", sngss7_intf->id);
			break;
		/**********************************************************************/
		case (SI_INTF_CONG2):
			SS7_DEBUG("State of ISUP intf %d = Congestion 2\n", sngss7_intf->id);
			break;
		/**********************************************************************/
		case (SI_INTF_CONG3):
			SS7_DEBUG("State of ISUP intf %d = Congestion 3\n", sngss7_intf->id);
			break;
		/**********************************************************************/
		default:
			/* should do something here to handle the possiblity of an unknown case */
			SS7_ERROR("Unknown ISUP intf Status code (%d) for Intf = %d\n", status, sngss7_intf->id);
			break;
		/**********************************************************************/
		} /* switch (status) */

	/**************************************************************************/
	} /* for (x = 1; x < MAX_ISUP_INFS); i++) */

	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t sngss7_add_var(sngss7_chan_data_t *sngss7_info, const char* var, const char* val)
{
	char	*t_name = 0;
	char	*t_val = 0;

	/* confirm the user has sent us a value */
	if (!var || !val) {
		return FTDM_FAIL;
	}

	if (!sngss7_info->variables) {
		/* initialize on first use */
		sngss7_info->variables = create_hashtable(16, ftdm_hash_hashfromstring, ftdm_hash_equalkeys);
		ftdm_assert_return(sngss7_info->variables, FTDM_FAIL, "Failed to create hash table\n");
	}

	t_name = ftdm_strdup(var);
	t_val = ftdm_strdup(val);

	hashtable_insert(sngss7_info->variables, t_name, t_val, HASHTABLE_FLAG_FREE_KEY | HASHTABLE_FLAG_FREE_VALUE);

	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t sngss7_add_raw_data(sngss7_chan_data_t *sngss7_info, uint8_t* data, ftdm_size_t data_len)
{
	ftdm_assert_return(!sngss7_info->raw_data, FTDM_FAIL, "Overwriting existing raw data\n");
	
	sngss7_info->raw_data = ftdm_calloc(1, data_len);
	ftdm_assert_return(sngss7_info->raw_data, FTDM_FAIL, "Failed to allocate raw data\n");

	memcpy(sngss7_info->raw_data, data, data_len);
	sngss7_info->raw_data_len = data_len;
	return FTDM_SUCCESS;
}

/******************************************************************************/
void sngss7_send_signal(sngss7_chan_data_t *sngss7_info, ftdm_signal_event_t event_id)
{
	ftdm_sigmsg_t	sigev;
	ftdm_channel_t	*ftdmchan = sngss7_info->ftdmchan;

	memset(&sigev, 0, sizeof(sigev));

	sigev.chan_id = ftdmchan->chan_id;
	sigev.span_id = ftdmchan->span_id;
	sigev.channel = ftdmchan;
	sigev.event_id = event_id;

	if (sngss7_info->variables) {
		/*
		* variables now belongs to the ftdm core, and
		* will be cleared after sigev is processed by user. Set
		* local pointer to NULL so we do not attempt to
		* destroy it */
		sigev.variables = sngss7_info->variables;
		sngss7_info->variables = NULL;
	}

	if (sngss7_info->raw_data) {
		/*
		* raw_data now belongs to the ftdm core, and
		* will be cleared after sigev is processed by user. Set
		* local pointer to NULL so we do not attempt to
		* destroy it */
		
		sigev.raw.data = sngss7_info->raw_data;
		sigev.raw.len = sngss7_info->raw_data_len;
		
		sngss7_info->raw_data = NULL;
		sngss7_info->raw_data_len = 0;
	}
	ftdm_span_send_signal(ftdmchan->span, &sigev);
}

/******************************************************************************/
void sngss7_set_sig_status(sngss7_chan_data_t *sngss7_info, ftdm_signaling_status_t status)
{
	ftdm_sigmsg_t	sig;
	ftdm_channel_t	*ftdmchan = sngss7_info->ftdmchan;

	ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "Signalling link status changed to %s\n",
								ftdm_signaling_status2str(status));
	
	memset(&sig, 0, sizeof(sig));

	sig.chan_id = ftdmchan->chan_id;
	sig.span_id = ftdmchan->span_id;
	sig.channel = ftdmchan;
	sig.event_id = FTDM_SIGEVENT_SIGSTATUS_CHANGED;
	sig.ev_data.sigstatus.status = status;

	if (ftdm_span_send_signal(ftdmchan->span, &sig) != FTDM_SUCCESS) {
		SS7_ERROR_CHAN(ftdmchan, "Failed to change channel status to %s\n", 
									ftdm_signaling_status2str(status));
	}
	return;
}

/******************************************************************************/
ftdm_status_t check_for_reconfig_flag(ftdm_span_t *ftdmspan)
{
	ftdm_channel_t		*ftdmchan = NULL;
	sngss7_chan_data_t	*sngss7_info = NULL;
	sng_isup_inf_t		*sngss7_intf = NULL;
	uint8_t				state;
	uint8_t				bits_ab = 0;
	uint8_t				bits_cd = 0;	
	uint8_t				bits_ef = 0;
	int 				x;
	int					ret;

	for (x = 1; x < (ftdmspan->chan_count + 1); x++) {
	/**************************************************************************/
		/* extract the channel structure and sngss7 channel data */
		ftdmchan = ftdmspan->channels[x];
		
		/* if the call data is NULL move on */
		if (ftdmchan->call_data == NULL) {
			SS7_WARN_CHAN(ftdmchan, "Found ftdmchan with no sig module data!%s\n", " ");
			continue;
		}

		/* grab the private data */
		sngss7_info = ftdmchan->call_data;

		/* check the reconfig flag */
		if (sngss7_test_ckt_flag(sngss7_info, FLAG_CKT_RECONFIG)) {
			/* confirm the state of all isup interfaces*/
			check_status_of_all_isup_intf();

			sngss7_intf = &g_ftdm_sngss7_data.cfg.isupIntf[sngss7_info->circuit->infId];

			/* check if the interface is paused or resumed */
			if (sngss7_test_flag(sngss7_intf, SNGSS7_PAUSED)) {
				ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "ISUP intf %d is PAUSED\n", sngss7_intf->id);
				/* throw the pause flag */
				sngss7_clear_ckt_flag(sngss7_info, FLAG_INFID_RESUME);
				sngss7_set_ckt_flag(sngss7_info, FLAG_INFID_PAUSED);
			} else {
				ftdm_log_chan(ftdmchan, FTDM_LOG_DEBUG, "ISUP intf %d is RESUMED\n", sngss7_intf->id);
				/* throw the resume flag */
				sngss7_clear_ckt_flag(sngss7_info, FLAG_INFID_PAUSED);
				sngss7_set_ckt_flag(sngss7_info, FLAG_INFID_RESUME);
			}

			/* query for the status of the ckt */
			if (ftmod_ss7_isup_ckt_sta(sngss7_info->circuit->id, &state)) {
				SS7_ERROR("Failed to read isup ckt = %d status\n", sngss7_info->circuit->id);
				continue;
			}

			/* extract the bit sections */
			bits_ab = (state & (SNG_BIT_A + SNG_BIT_B)) >> 0;
			bits_cd = (state & (SNG_BIT_C + SNG_BIT_D)) >> 2;
			bits_ef = (state & (SNG_BIT_E + SNG_BIT_F)) >> 4;

			if (bits_cd == 0x0) {
				/* check if circuit is UCIC or transient */
				if (bits_ab == 0x3) {
					/* bit a and bit b are set, unequipped */
					ret = ftmod_ss7_isup_ckt_config(sngss7_info->circuit->id);
					if (ret) {
						SS7_CRITICAL("ISUP CKT %d re-configuration FAILED!\n",x);
					} else {
						SS7_INFO("ISUP CKT %d re-configuration DONE!\n", x);
					}

					/* reset the circuit to sync states */
					ftdm_mutex_lock(ftdmchan->mutex);
			
					/* flag the circuit as active */
					sngss7_set_flag(sngss7_info->circuit, SNGSS7_ACTIVE);

					/* throw the channel into reset */
					sngss7_set_ckt_flag(sngss7_info, FLAG_RESET_TX);

					/* throw the channel to suspend */
					ftdm_set_state(ftdmchan, FTDM_CHANNEL_STATE_SUSPENDED);
			
					/* unlock the channel */
					ftdm_mutex_unlock(ftdmchan->mutex);

				} /* if (bits_ab == 0x3) */
			} else {
				/* check the maintenance block status in bits A and B */
				switch (bits_ab) {
				/**************************************************************************/
				case (0):
					/* no maintenace block...do nothing */
					break;
				/**************************************************************************/
				case (1):
					/* locally blocked */
					sngss7_set_ckt_blk_flag(sngss7_info, FLAG_CKT_LC_BLOCK_RX);

					/* set the channel to suspended state */
					SS7_STATE_CHANGE(ftdmchan, FTDM_CHANNEL_STATE_SUSPENDED);
					break;
				/**************************************************************************/
				case (2):
					/* remotely blocked */
					sngss7_set_ckt_blk_flag(sngss7_info, FLAG_CKT_MN_BLOCK_RX);

					/* set the channel to suspended state */
					SS7_STATE_CHANGE(ftdmchan, FTDM_CHANNEL_STATE_SUSPENDED);
					break;
				/**************************************************************************/
				case (3):
					/* both locally and remotely blocked */
					sngss7_set_ckt_blk_flag(sngss7_info, FLAG_CKT_LC_BLOCK_RX);
					sngss7_set_ckt_blk_flag(sngss7_info, FLAG_CKT_MN_BLOCK_RX);

					/* set the channel to suspended state */
					SS7_STATE_CHANGE(ftdmchan, FTDM_CHANNEL_STATE_SUSPENDED);
					break;
				/**************************************************************************/
				default:
					break;
				/**************************************************************************/
				} /* switch (bits_ab) */
			
				/* check the hardware block status in bits e and f */
				switch (bits_ef) {
				/**************************************************************************/
				case (0):
					/* no maintenace block...do nothing */
					break;
				/**************************************************************************/
				case (1):
					/* locally blocked */
					sngss7_set_ckt_blk_flag(sngss7_info, FLAG_GRP_HW_BLOCK_TX);

					/* set the channel to suspended state */
					SS7_STATE_CHANGE(ftdmchan, FTDM_CHANNEL_STATE_SUSPENDED);
					break;
				/**************************************************************************/
				case (2):
					/* remotely blocked */
					sngss7_set_ckt_blk_flag(sngss7_info, FLAG_GRP_HW_BLOCK_RX);

					/* set the channel to suspended state */
					SS7_STATE_CHANGE(ftdmchan, FTDM_CHANNEL_STATE_SUSPENDED);
					break;
				/**************************************************************************/
				case (3):
					/* both locally and remotely blocked */
					sngss7_set_ckt_blk_flag(sngss7_info, FLAG_GRP_HW_BLOCK_TX);
					sngss7_set_ckt_blk_flag(sngss7_info, FLAG_GRP_HW_BLOCK_RX);

					/* set the channel to suspended state */
					SS7_STATE_CHANGE(ftdmchan, FTDM_CHANNEL_STATE_SUSPENDED);
					break;
				/**************************************************************************/
				default:
					break;
				/**************************************************************************/
				} /* switch (bits_ef) */
			}

			/* clear the re-config flag ... no matter what */
			sngss7_clear_ckt_flag(sngss7_info, FLAG_CKT_RECONFIG);

		} /* if ((sngss7_test_ckt_flag(sngss7_info, FLAG_CKT_RECONFIG)) */
	} /* for (x = 1; x < (span->chan_count + 1); x++) */

	return FTDM_SUCCESS;
}
/******************************************************************************/
/* For Emacs:
 * Local Variables:
 * mode:c
 * indent-tabs-mode:t
 * tab-width:4
 * c-basic-offset:4
 * End:
 * For VIM:
 * vim:set softtabstop=4 shiftwidth=4 tabstop=4:
 */
/******************************************************************************/
