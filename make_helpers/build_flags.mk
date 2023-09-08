ifneq (${ENABLE_SYS_REG_TRACE_FOR_NS},)
$(warning As part of an effort to unify the build flag naming \
	conventions, ENABLE_SYS_REG_TRACE_FOR_NS will soon be deprecated. \
	The updated version of this flag is ENABLE_SYS_REG_TRACE.")
ENABLE_SYS_REG_TRACE	:= ENABLE_SYS_REG_TRACE_FOR_NS
endif #(ENABLE_SYS_REG_TRACE_FOR_NS)
