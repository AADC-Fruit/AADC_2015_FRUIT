# search for NFQCA lib
find_path ( NFQCA_INCLUDE_DIR nfqcaCritic.h nfqcaActor.h HINTS /usr/include ${NFQCA_DIR}/include)
find_library ( NFQCACritic_LIBRARY NAMES  nfqcaCritic
             HINTS /usr/lib ${NFQCA_DIR}/lib)
find_library ( NFQCAActor_LIBRARY NAMES  nfqcaActor
             HINTS /usr/lib ${NFQCA_DIR}/lib)
if ( NFQCACritic_LIBRARY AND NFQCAActor_LIBRARY AND NFQCA_INCLUDE_DIR )
  set ( FOUND_NFQCA TRUE )
elseif ( NOT NFQCA_FIND_QUIETLY )
#  message ( WARNING " could not find NFQCA (LIBRARY=${NFQCAActor_LIBRARY}, INCLUDE=${NFQCA_INCLUDE_DIR}). You might want to specify NFQCA_DIR using -DNFQCA_DIR=..." )
  message ( WARNING " could not find NFQCA includes and libraries. You might want to specify NFQCA_DIR using -DNFQCA_DIR=..." )
endif ()
mark_as_advanced ( NFQCA_INCLUDE_DIR NFQCAActor_LIBRARY NFQCACritic_LIBRARY )
