APP=${1}
. common/apps/${APP}/Makefile

VENDOR=$2
. device/${VENDOR}/instamsg/Makefile


TOTAL_INCLUDES=`echo                                                                            \
        ${SYSTEM_INCLUDES}                                                                      \
        -Idevice/${VENDOR}/instamsg/                                                            \
        -Idevice/${VENDOR}/ioeye/                                                               \
        -Icommon/instamsg/mqtt/                                                                 `


SOURCES=`echo                                                                                   \
        ${APP_SOURCES}                                                                          \
        ${EXTRA_SOURCES}                                                                        \
        common/instamsg/driver/instamsg.c                                                       \
        common/instamsg/driver/log.c                                                            \
        common/instamsg/driver/json.c                                                           \
        common/instamsg/driver/globals.c                                                        \
        common/instamsg/driver/sg_mem.c                                                         \
        common/instamsg/driver/sg_stdlib.c                                                      \
        common/instamsg/driver/socket.c                                                         \
        common/instamsg/driver/config.c                                                         \
        common/instamsg/driver/watchdog.c                                                       \
        common/instamsg/driver/file_system.c                                                    \
        common/instamsg/driver/httpclient.c                                                     \
        common/instamsg/driver/media.c                                                          \
        common/instamsg/driver/upgrade.c                                                        \
        common/instamsg/driver/gps.c                                                            \
        common/instamsg/driver/time.c                                                           \
        common/instamsg/driver/hex.c                                                            \
        common/instamsg/driver/misc.c                                                           \
                                                                        \
        common/instamsg/openssl/crypto/aes/aes_ige.c		\
        common/instamsg/openssl/crypto/aes/aes_cbc.c		\
        common/instamsg/openssl/crypto/aes/aes_ecb.c		\
        common/instamsg/openssl/crypto/aes/aes_wrap.c		\
        common/instamsg/openssl/crypto/aes/aes_misc.c		\
        common/instamsg/openssl/crypto/aes/aes_cfb.c		\
        common/instamsg/openssl/crypto/aes/aes_ofb.c		\
        common/instamsg/openssl/crypto/aes/aes_ctr.c		\
        common/instamsg/openssl/crypto/aes/aes_core.c		\
        common/instamsg/openssl/crypto/aes/aes_x86core.c		\
        common/instamsg/openssl/crypto/bf/bf_opts.c		\
        common/instamsg/openssl/crypto/bf/bf_ofb64.c		\
        common/instamsg/openssl/crypto/bf/bftest.c		\
        common/instamsg/openssl/crypto/bf/bf_skey.c		\
        common/instamsg/openssl/crypto/bf/bf_cfb64.c		\
        common/instamsg/openssl/crypto/bf/bf_enc.c		\
        common/instamsg/openssl/crypto/bf/bf_ecb.c		\
        common/instamsg/openssl/crypto/bf/bfspeed.c		\
        common/instamsg/openssl/crypto/bf/bf_cbc.c		\
        common/instamsg/openssl/crypto/bio/bss_bio.c		\
        common/instamsg/openssl/crypto/bio/bio_lib.c		\
        common/instamsg/openssl/crypto/bio/bss_null.c		\
        common/instamsg/openssl/crypto/bio/bf_nbio.c		\
        common/instamsg/openssl/crypto/bio/bio_err.c		\
        common/instamsg/openssl/crypto/bio/bss_rtcp.c		\
        common/instamsg/openssl/crypto/bio/bss_sock.c		\
        common/instamsg/openssl/crypto/bio/bf_lbuf.c		\
        common/instamsg/openssl/crypto/bio/bss_conn.c		\
        common/instamsg/openssl/crypto/bio/bss_dgram.c		\
        common/instamsg/openssl/crypto/bio/bss_log.c		\
        common/instamsg/openssl/crypto/bio/b_dump.c		\
        common/instamsg/openssl/crypto/bio/b_sock.c		\
        common/instamsg/openssl/crypto/bio/bss_fd.c		\
        common/instamsg/openssl/crypto/bio/bss_acpt.c		\
        common/instamsg/openssl/crypto/bio/bio_cb.c		\
        common/instamsg/openssl/crypto/bio/b_print.c		\
        common/instamsg/openssl/crypto/bio/bss_mem.c		\
        common/instamsg/openssl/crypto/bio/bss_file.c		\
        common/instamsg/openssl/crypto/bio/bf_null.c		\
        common/instamsg/openssl/crypto/bio/bf_buff.c		\
        common/instamsg/openssl/crypto/bn/bn_lib.c		\
        common/instamsg/openssl/crypto/bn/bn_blind.c		\
        common/instamsg/openssl/crypto/bn/bn_shift.c		\
        common/instamsg/openssl/crypto/bn/bn_mul.c		\
        common/instamsg/openssl/crypto/bn/bn_nist.c		\
        common/instamsg/openssl/crypto/bn/bn_sqrt.c		\
        common/instamsg/openssl/crypto/bn/bn_prime.c		\
        common/instamsg/openssl/crypto/bn/bn_word.c		\
        common/instamsg/openssl/crypto/bn/vms-helper.c		\
                                                                                            \
		common/instamsg/openssl/crypto/bn/bnspeed.c		\
		common/instamsg/openssl/crypto/bn/bn_x931p.c		\
		common/instamsg/openssl/crypto/bn/bn_err.c		\
		common/instamsg/openssl/crypto/bn/exptest.c		\
		common/instamsg/openssl/crypto/bn/bn_gcd.c		\
		common/instamsg/openssl/crypto/bn/bn_depr.c		\
		common/instamsg/openssl/crypto/bn/bn_mpi.c		\
		common/instamsg/openssl/crypto/bn/bn_exp.c		\
		common/instamsg/openssl/crypto/bn/bn_print.c		\
		common/instamsg/openssl/crypto/bn/bn_const.c		\
		common/instamsg/openssl/crypto/bn/expspeed.c		\
		common/instamsg/openssl/crypto/bn/bn_mont.c		\
		common/instamsg/openssl/crypto/bn/bn_exp2.c		\
		common/instamsg/openssl/crypto/bn/bn_recp.c		\
		common/instamsg/openssl/crypto/bn/rsaz_exp.c		\
		common/instamsg/openssl/crypto/bn/exp.c		\
		common/instamsg/openssl/crypto/bn/bn_ctx.c		\
		common/instamsg/openssl/crypto/bn/bn_sqr.c		\
		common/instamsg/openssl/crypto/bn/bn_add.c		\
		common/instamsg/openssl/crypto/bn/bn_gf2m.c		\
		common/instamsg/openssl/crypto/bn/bn_asm.c		\
		common/instamsg/openssl/crypto/bn/divtest.c		\
		common/instamsg/openssl/crypto/bn/bntest.c		\
		common/instamsg/openssl/crypto/bn/bn_mod.c		\
		common/instamsg/openssl/crypto/bn/bn_kron.c		\
		common/instamsg/openssl/crypto/bn/bn_div.c		\
		common/instamsg/openssl/crypto/bn/bn_rand.c		\
		common/instamsg/openssl/crypto/buffer/buf_str.c		\
		common/instamsg/openssl/crypto/buffer/buffer.c		\
		common/instamsg/openssl/crypto/buffer/buf_err.c		\
		common/instamsg/openssl/crypto/cmac/cm_pmeth.c		\
		common/instamsg/openssl/crypto/cmac/cm_ameth.c		\
		common/instamsg/openssl/crypto/cmac/cmac.c		\
		common/instamsg/openssl/crypto/comp/comp_err.c		\
		common/instamsg/openssl/crypto/comp/comp_lib.c		\
		common/instamsg/openssl/crypto/comp/c_zlib.c		\
		common/instamsg/openssl/crypto/comp/c_rle.c		\
		common/instamsg/openssl/crypto/conf/conf_mall.c		\
		common/instamsg/openssl/crypto/conf/conf_api.c		\
		common/instamsg/openssl/crypto/conf/test.c		\
        common/instamsg/openssl/crypto/conf/conf_err.c		\
		common/instamsg/openssl/crypto/conf/conf_def.c		\
		common/instamsg/openssl/crypto/conf/conf_mod.c		\
		common/instamsg/openssl/crypto/conf/conf_sap.c		\
		common/instamsg/openssl/crypto/conf/cnf_save.c		\
		common/instamsg/openssl/crypto/conf/conf_lib.c		\
		common/instamsg/openssl/crypto/des/des_old2.c		\
		common/instamsg/openssl/crypto/des/set_key.c		\
		common/instamsg/openssl/crypto/des/cfb_enc.c		\
		common/instamsg/openssl/crypto/des/xcbc_enc.c		\
		common/instamsg/openssl/crypto/des/des_enc.c		\
		common/instamsg/openssl/crypto/des/ofb64ede.c		\
		common/instamsg/openssl/crypto/des/cfb64enc.c		\
		common/instamsg/openssl/crypto/des/cbc_cksm.c		\
		common/instamsg/openssl/crypto/des/pcbc_enc.c		\
		common/instamsg/openssl/crypto/des/destest.c		\
		common/instamsg/openssl/crypto/des/ecb_enc.c		\
		common/instamsg/openssl/crypto/des/ofb_enc.c		\
		common/instamsg/openssl/crypto/des/cbc_enc.c		\
		common/instamsg/openssl/crypto/des/cbc3_enc.c		\
		common/instamsg/openssl/crypto/des/read2pwd.c		\
		common/instamsg/openssl/crypto/des/str2key.c		\
		common/instamsg/openssl/crypto/des/speed.c		\
		common/instamsg/openssl/crypto/des/des.c		\
		common/instamsg/openssl/crypto/des/ofb64enc.c		\
		common/instamsg/openssl/crypto/des/ecb3_enc.c		\
		common/instamsg/openssl/crypto/des/rpw.c		\
		common/instamsg/openssl/crypto/des/des_old.c		\
		common/instamsg/openssl/crypto/des/des_opts.c		\
		common/instamsg/openssl/crypto/des/fcrypt.c		\
		common/instamsg/openssl/crypto/des/rand_key.c		\
		common/instamsg/openssl/crypto/des/qud_cksm.c		\
		common/instamsg/openssl/crypto/des/ede_cbcm_enc.c		\
		common/instamsg/openssl/crypto/des/fcrypt_b.c		\
		common/instamsg/openssl/crypto/des/cfb64ede.c		\
		common/instamsg/openssl/crypto/des/ncbc_enc.c		\
		common/instamsg/openssl/crypto/des/read_pwd.c		\
		common/instamsg/openssl/crypto/des/rpc_enc.c		\
		common/instamsg/openssl/crypto/des/enc_writ.c		\
		common/instamsg/openssl/crypto/des/enc_read.c		\
		common/instamsg/openssl/crypto/dso/dso_beos.c		\
		common/instamsg/openssl/crypto/dso/dso_lib.c		\
		common/instamsg/openssl/crypto/dso/dso_dlfcn.c		\
		common/instamsg/openssl/crypto/dso/dso_vms.c		\
		common/instamsg/openssl/crypto/dso/dso_openssl.c		\
		common/instamsg/openssl/crypto/dso/dso_dl.c		\
		common/instamsg/openssl/crypto/dso/dso_win32.c		\
		common/instamsg/openssl/crypto/dso/dso_null.c		\
		common/instamsg/openssl/crypto/dso/dso_err.c		\
		common/instamsg/openssl/crypto/ec/eck_prn.c		\
		common/instamsg/openssl/crypto/ec/ectest.c		\
		common/instamsg/openssl/crypto/ec/ecp_nistputil.c		\
		common/instamsg/openssl/crypto/ec/ec_err.c		\
		common/instamsg/openssl/crypto/ec/ecp_nistp521.c		\
		common/instamsg/openssl/crypto/ec/ec_curve.c		\
		common/instamsg/openssl/crypto/ec/ecp_nistp224.c		\
		common/instamsg/openssl/crypto/ec/ecp_nist.c		\
		common/instamsg/openssl/crypto/ec/ec_ameth.c		\
		common/instamsg/openssl/crypto/ec/ec2_oct.c		\
		common/instamsg/openssl/crypto/ec/ec_mult.c		\
		common/instamsg/openssl/crypto/ec/ecp_nistp256.c		\
		common/instamsg/openssl/crypto/ec/ec_key.c		\
		common/instamsg/openssl/crypto/ec/ec_pmeth.c		\
		common/instamsg/openssl/crypto/ec/ec_asn1.c		\
		common/instamsg/openssl/crypto/ec/ec_cvt.c		\
		common/instamsg/openssl/crypto/ec/ec2_smpl.c		\
		common/instamsg/openssl/crypto/ec/ec_lib.c		\
		common/instamsg/openssl/crypto/ec/ecp_mont.c		\
		common/instamsg/openssl/crypto/ec/ecp_smpl.c		\
		common/instamsg/openssl/crypto/ec/ecp_oct.c		\
		common/instamsg/openssl/crypto/ec/ecp_nistz256_table.c		\
		common/instamsg/openssl/crypto/ec/ec_print.c		\
		common/instamsg/openssl/crypto/ec/ec_oct.c		\
		common/instamsg/openssl/crypto/ec/ecp_nistz256.c		\
		common/instamsg/openssl/crypto/ec/ec_check.c		\
		common/instamsg/openssl/crypto/ec/ec2_mult.c		\
		common/instamsg/openssl/crypto/ecdh/ech_kdf.c		\
		common/instamsg/openssl/crypto/ecdh/ech_ossl.c		\
		common/instamsg/openssl/crypto/ecdh/ecdhtest.c		\
		common/instamsg/openssl/crypto/ecdh/ech_err.c		\
		common/instamsg/openssl/crypto/ecdh/ech_key.c		\
		common/instamsg/openssl/crypto/ecdh/ech_lib.c		\
		common/instamsg/openssl/crypto/ecdsa/ecs_vrf.c		\
		common/instamsg/openssl/crypto/ecdsa/ecdsatest.c		\
		common/instamsg/openssl/crypto/ecdsa/ecs_ossl.c		\
		common/instamsg/openssl/crypto/ecdsa/ecs_sign.c		\
		common/instamsg/openssl/crypto/ecdsa/ecs_lib.c		\
		common/instamsg/openssl/crypto/ecdsa/ecs_err.c		\
		common/instamsg/openssl/crypto/ecdsa/ecs_asn1.c		\
		common/instamsg/openssl/crypto/engine/tb_rsa.c		\
		common/instamsg/openssl/crypto/engine/eng_pkey.c		\
		common/instamsg/openssl/crypto/engine/tb_digest.c		\
		common/instamsg/openssl/crypto/engine/tb_pkmeth.c		\
		common/instamsg/openssl/crypto/engine/eng_err.c		\
		common/instamsg/openssl/crypto/engine/eng_all.c		\
		common/instamsg/openssl/crypto/engine/tb_asnmth.c		\
		common/instamsg/openssl/crypto/engine/tb_dh.c		\
		common/instamsg/openssl/crypto/engine/eng_ctrl.c		\
		common/instamsg/openssl/crypto/engine/eng_lib.c		\
		common/instamsg/openssl/crypto/engine/eng_fat.c		\
		common/instamsg/openssl/crypto/engine/tb_cipher.c		\
		common/instamsg/openssl/crypto/engine/eng_openssl.c		\
		common/instamsg/openssl/crypto/engine/eng_init.c		\
		common/instamsg/openssl/crypto/engine/tb_store.c		\
		common/instamsg/openssl/crypto/engine/eng_rdrand.c		\
		common/instamsg/openssl/crypto/engine/eng_cnf.c		\
		common/instamsg/openssl/crypto/engine/eng_dyn.c		\
		common/instamsg/openssl/crypto/engine/eng_table.c		\
		common/instamsg/openssl/crypto/engine/eng_list.c		\
		common/instamsg/openssl/crypto/engine/enginetest.c		\
		common/instamsg/openssl/crypto/engine/tb_ecdsa.c		\
		common/instamsg/openssl/crypto/engine/tb_ecdh.c		\
		common/instamsg/openssl/crypto/engine/eng_cryptodev.c		\
		common/instamsg/openssl/crypto/engine/tb_dsa.c		\
		common/instamsg/openssl/crypto/engine/tb_rand.c		\
		common/instamsg/openssl/crypto/hmac/hmactest.c		\
		common/instamsg/openssl/crypto/hmac/hm_ameth.c		\
		common/instamsg/openssl/crypto/hmac/hmac.c		\
		common/instamsg/openssl/crypto/hmac/hm_pmeth.c		\
		common/instamsg/openssl/crypto/lhash/lh_stats.c		\
        common/instamsg/openssl/crypto/lhash/lh_test.c		\
		common/instamsg/openssl/crypto/lhash/lhash.c		\
		common/instamsg/openssl/crypto/modes/ccm128.c		\
		common/instamsg/openssl/crypto/modes/wrap128.c		\
		common/instamsg/openssl/crypto/modes/ofb128.c		\
		common/instamsg/openssl/crypto/modes/cbc128.c		\
		common/instamsg/openssl/crypto/modes/gcm128.c		\
		common/instamsg/openssl/crypto/modes/ctr128.c		\
		common/instamsg/openssl/crypto/modes/cfb128.c		\
		common/instamsg/openssl/crypto/modes/xts128.c		\
		common/instamsg/openssl/crypto/modes/cts128.c		\
		common/instamsg/openssl/crypto/objects/obj_dat.c		\
		common/instamsg/openssl/crypto/objects/obj_xref.c		\
		common/instamsg/openssl/crypto/objects/obj_err.c		\
		common/instamsg/openssl/crypto/objects/obj_lib.c		\
		common/instamsg/openssl/crypto/objects/o_names.c		\
		common/instamsg/openssl/crypto/ocsp/ocsp_asn.c		\
		common/instamsg/openssl/crypto/ocsp/ocsp_ht.c		\
		common/instamsg/openssl/crypto/ocsp/ocsp_cl.c		\
		common/instamsg/openssl/crypto/ocsp/ocsp_ext.c		\
		common/instamsg/openssl/crypto/ocsp/ocsp_vfy.c		\
		common/instamsg/openssl/crypto/ocsp/ocsp_prn.c		\
		common/instamsg/openssl/crypto/ocsp/ocsp_lib.c		\
		common/instamsg/openssl/crypto/ocsp/ocsp_err.c		\
		common/instamsg/openssl/crypto/ocsp/ocsp_srv.c		\
		common/instamsg/openssl/crypto/stack/stack.c		\
                                                                                            \
        common/instamsg/mqtt/src/MQTTFormat.c                                                   \
        common/instamsg/mqtt/src/MQTTPacket.c                                                   \
        common/instamsg/mqtt/src/MQTTDeserializePublish.c                                       \
        common/instamsg/mqtt/src/MQTTConnectClient.c                                            \
        common/instamsg/mqtt/src/MQTTSubscribeClient.c                                          \
        common/instamsg/mqtt/src/MQTTSerializePublish.c                                         \
        common/instamsg/mqtt/src/MQTTConnectServer.c                                            \
        common/instamsg/mqtt/src/MQTTSubscribeServer.c                                          \
        common/instamsg/mqtt/src/MQTTUnsubscribeServer.c                                        \
        common/instamsg/mqtt/src/MQTTUnsubscribeClient.c                                        \
                                                                                                \
        device/${VENDOR}/instamsg/device_socket.c                                               \
        device/${VENDOR}/instamsg/device_serial_logger.c                                        \
        device/${VENDOR}/instamsg/device_time.c                                                 \
        device/${VENDOR}/instamsg/device_watchdog.c                                             \
        device/${VENDOR}/instamsg/device_config.c                                               \
        device/${VENDOR}/instamsg/device_media.c                                                \
        device/${VENDOR}/instamsg/device_file_system.c                                          \
        device/${VENDOR}/instamsg/device_data_logger.c                                          \
        device/${VENDOR}/instamsg/device_upgrade.c                                              \
        device/${VENDOR}/instamsg/device_gps.c                                                  \
        device/${VENDOR}/instamsg/device_misc.c                                                 `


SOURCES=`echo ${SOURCES} | sed -e 's|VENDOR|'"${VENDOR}"'|g'`

OUT_DIR="setups/${VENDOR}"
mkdir -p ${OUT_DIR}


COMPILE_COMMAND="${COMPILE_COMMAND} ${APP_COMPILER_DIRECTIVES} "

INSTAMSG_VERSION=`grep INSTAMSG_VERSION common/instamsg/driver/include/globals.h | cut -d\  -f 3 | cut -d\" -f 2`
DEVICE_VERSION=`grep DEVICE_VERSION device/${VENDOR}/instamsg/device_defines.h | cut -d\  -f 3 | cut -d\" -f 2`
COMPILED_FILE_NAME="${APP}_${VENDOR}_${INSTAMSG_VERSION}_${DEVICE_VERSION}"

TOTAL_COMPILE_COMMAND="${COMPILE_COMMAND} ${TOTAL_INCLUDES} ${SOURCES} -o ${OUT_DIR}/${COMPILED_FILE_NAME}"

for obj in ${EXTRA_OBJECT_FILES}
do
    TOTAL_COMPILE_COMMAND="${COMPILE_COMMAND} ${TOTAL_INCLUDES} ${SOURCES} ${OUT_DIR}/* -o ${OUT_DIR}/${COMPILED_FILE_NAME}"
    cp ${obj} ${OUT_DIR}
done

TOTAL_COMPILE_COMMAND="${TOTAL_COMPILE_COMMAND} ${POST_LINKED_LIBS} "
${TOTAL_COMPILE_COMMAND}

FINAL_FILE_NAME="${COMPILED_FILE_NAME}_to-flash"
for cmd in "${FINAL_COMMANDS[@]}"
do
    cmd=`echo ${cmd} | sed -e 's|OUT_DIR|'"${OUT_DIR}"'|g'`
    cmd=`echo ${cmd} | sed -e 's|COMPILED_FILE_NAME|'"${COMPILED_FILE_NAME}"'|g'`
    cmd=`echo ${cmd} | sed -e 's|FINAL_FILE_NAME|'"${FINAL_FILE_NAME}"'|g'`

    $cmd
done
