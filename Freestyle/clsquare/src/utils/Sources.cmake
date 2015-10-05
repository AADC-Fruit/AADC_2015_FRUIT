LIST(APPEND utils_srcs
	${CLSQUARE_SOURCE_DIR}/utils/aset.cc
	${CLSQUARE_SOURCE_DIR}/utils/filter.cc
	${CLSQUARE_SOURCE_DIR}/utils/cls_random.cc
	${CLSQUARE_SOURCE_DIR}/utils/sysmodel.cc
	${CLSQUARE_SOURCE_DIR}/utils/tcpsocket.cc
	${CLSQUARE_SOURCE_DIR}/utils/udpsocket.cc
	${CLSQUARE_SOURCE_DIR}/utils/costfunc.cc
  ${CLSQUARE_SOURCE_DIR}/utils/Socket.cc
  ${CLSQUARE_SOURCE_DIR}/utils/funcgen1d.cc
  ${CLSQUARE_SOURCE_DIR}/utils/setdef.cc
  ${CLSQUARE_SOURCE_DIR}/utils/str2val.cc
	${CLSQUARE_SOURCE_DIR}/utils/BatchUtils/BatchData.cc
	${CLSQUARE_SOURCE_DIR}/utils/BatchUtils/AbstractBatchController.cc
	${CLSQUARE_SOURCE_DIR}/utils/BatchUtils/VectorArithmetic.cc
)

include(${CLSQUARE_SOURCE_DIR}/utils/ImageSourceUtils/Sources.cmake)
