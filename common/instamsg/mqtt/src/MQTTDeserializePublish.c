/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *******************************************************************************/

#include "StackTrace.h"
#include "MQTTPacket.h"
#include <string.h>

int MQTTDeserialize_publish(MQTTFixedHeaderPlusMsgId *fixedHeaderPlusMsgId,
                            MQTTString* topicName,
                            unsigned char** payload,
                            int* payloadlen,
                            unsigned char* buf,
                            int buflen)
{
	MQTTHeader header = {0};
	unsigned char* curdata = buf;
	unsigned char* enddata = NULL;
	int rc = FAILURE;
	int mylen = 0;

	FUNC_ENTRY;
	header.byte = readChar(&curdata);
	if (header.bits.type != PUBLISH)
		goto exit;

    fillFixedHeaderFieldsFromPacketHeader(&(fixedHeaderPlusMsgId->fixedHeader), &header);

	curdata += (rc = MQTTPacket_decodeBuf(curdata, &mylen)); /* read remaining length */
	enddata = curdata + mylen;

	if (!readMQTTLenString(topicName, &curdata, enddata) ||
		enddata - curdata < 0) /* do we have enough data to read the protocol version byte? */
		goto exit;

	if ((fixedHeaderPlusMsgId->fixedHeader).qos > 0)
		fixedHeaderPlusMsgId->msgId = readInt(&curdata);

	*payloadlen = enddata - curdata;
	*payload = curdata;

	rc = SUCCESS;

exit:
	FUNC_EXIT_RC(rc);
	return rc;
}


int MQTTDeserialize_FixedHeaderAndMsgId(MQTTFixedHeaderPlusMsgId *fixedHeaderPlusMsgId, unsigned char* buf, int buflen)
{
	MQTTHeader header = {0};
	unsigned char* curdata = buf;
	unsigned char* enddata = NULL;
	int rc = FAILURE;
	int mylen;

	FUNC_ENTRY;

	header.byte = readChar(&curdata);
    fillFixedHeaderFieldsFromPacketHeader(&(fixedHeaderPlusMsgId->fixedHeader), &header);

	curdata += (rc = MQTTPacket_decodeBuf(curdata, &mylen)); /* read remaining length */
	enddata = curdata + mylen;

	if (enddata - curdata < 2)
		goto exit;

	fixedHeaderPlusMsgId->msgId = readInt(&curdata);
	rc = SUCCESS;

exit:
	FUNC_EXIT_RC(rc);
	return rc;
}

