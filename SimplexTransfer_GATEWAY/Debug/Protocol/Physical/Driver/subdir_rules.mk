################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
Protocol/Physical/Driver/CC1101.obj: ../Protocol/Physical/Driver/CC1101.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.3/bin/cl430" -vmsp --abi=eabi --use_hw_mpy=none --preinclude="C:/Users/Mony/workspace_v6_0/SimplexTransfer_GATEWAY/Application/Gateway/SimplexTransferLR09Config.h" --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.3/include" --include_path="C:/Users/Mony/workspace_v6_0/SimplexTransfer_GATEWAY/Application" --include_path="C:/Users/Mony/workspace_v6_0/SimplexTransfer_GATEWAY/Protocol" --include_path="C:/Users/Mony/workspace_v6_0/SimplexTransfer_GATEWAY/Protocol/API" --include_path="C:/Users/Mony/workspace_v6_0/SimplexTransfer_GATEWAY/Protocol/DataLink" --include_path="C:/Users/Mony/workspace_v6_0/SimplexTransfer_GATEWAY/Protocol/DataLink/MAC" --include_path="C:/Users/Mony/workspace_v6_0/SimplexTransfer_GATEWAY/Protocol/DataLink/PhyBridge" --include_path="C:/Users/Mony/workspace_v6_0/SimplexTransfer_GATEWAY/Protocol/Physical" --include_path="C:/Users/Mony/workspace_v6_0/SimplexTransfer_GATEWAY/Protocol/Physical/Driver" --include_path="C:/Users/Mony/workspace_v6_0/SimplexTransfer_GATEWAY/Protocol/Physical/Module" --include_path="C:/Users/Mony/workspace_v6_0/SimplexTransfer_GATEWAY/Protocol/Physical/Module/A110LR09" --include_path="C:/Users/Mony/workspace_v6_0/SimplexTransfer_GATEWAY/Protocol/Physical/PhyBridge" --advice:power=all -g --define=__MSP430G2553__ --diag_warning=225 --display_error_number --diag_wrap=off --printf_support=minimal --preproc_with_compile --preproc_dependency="Protocol/Physical/Driver/CC1101.pp" --obj_directory="Protocol/Physical/Driver" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


