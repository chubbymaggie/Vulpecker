int
CVE_2010_2248_linux2_6_15_CIFSSMBWrite2(const int xid, struct cifsTconInfo *tcon,
	     const int netfid, const unsigned int count,
	     const __u64 offset, unsigned int *nbytes, struct kvec *iov,
	     int n_vec, const int long_op)
{
	int rc = -EACCES;
	WRITE_REQ *pSMB = NULL;
	int bytes_returned, wct;
	int smb_hdr_len;

	/* BB removeme BB */
	cFYI(1,("write2 at %lld %d bytes", (long long)offset, count));

	if(tcon->ses->capabilities & CAP_LARGE_FILES)
		wct = 14;
	else
		wct = 12;
	rc = small_smb_init(SMB_COM_WRITE_ANDX, wct, tcon, (void **) &pSMB);
	if (rc)
		return rc;
	/* tcon and ses pointer are checked in smb_init */
	if (tcon->ses->server == NULL)
		return -ECONNABORTED;

	pSMB->AndXCommand = 0xFF;	/* none */
	pSMB->Fid = netfid;
	pSMB->OffsetLow = cpu_to_le32(offset & 0xFFFFFFFF);
	if(wct == 14)
		pSMB->OffsetHigh = cpu_to_le32(offset >> 32);
	else if((offset >> 32) > 0) /* can not handle this big offset for old */
		return -EIO;
	pSMB->Reserved = 0xFFFFFFFF;
	pSMB->WriteMode = 0;
	pSMB->Remaining = 0;

	pSMB->DataOffset =
	    cpu_to_le16(offsetof(struct smb_com_write_req,Data) - 4);

	pSMB->DataLengthLow = cpu_to_le16(count & 0xFFFF);
	pSMB->DataLengthHigh = cpu_to_le16(count >> 16);
	smb_hdr_len = pSMB->hdr.smb_buf_length + 1; /* hdr + 1 byte pad */
	if(wct == 14)
		pSMB->hdr.smb_buf_length += count+1;
	else /* wct == 12 */
		pSMB->hdr.smb_buf_length += count+5; /* smb data starts later */ 
	if(wct == 14)
		pSMB->ByteCount = cpu_to_le16(count + 1);
	else /* wct == 12 */ /* bigger pad, smaller smb hdr, keep offset ok */ {
		struct smb_com_writex_req * pSMBW =
				(struct smb_com_writex_req *)pSMB;
		pSMBW->ByteCount = cpu_to_le16(count + 5);
	}
	iov[0].iov_base = pSMB;
	iov[0].iov_len = smb_hdr_len + 4;

	rc = SendReceive2(xid, tcon->ses, iov, n_vec + 1, &bytes_returned,
			  long_op);
	cifs_stats_inc(&tcon->num_writes);
	if (rc) {
		cFYI(1, ("Send error Write2 = %d", rc));
		*nbytes = 0;
	} else {
		WRITE_RSP * pSMBr = (WRITE_RSP *)pSMB;
		*nbytes = le16_to_cpu(pSMBr->CountHigh);
		*nbytes = (*nbytes) << 16;
		*nbytes += le16_to_cpu(pSMBr->Count);
	}

	cifs_small_buf_release(pSMB);

	/* Note: On -EAGAIN error only caller can retry on handle based calls 
		since file handle passed in no longer valid */

	return rc;
}