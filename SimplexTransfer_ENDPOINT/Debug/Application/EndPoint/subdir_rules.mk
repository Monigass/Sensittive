################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
Application/EndPoint/SimplexTransfer.obj: ../Application/EndPoint/SimplexTransfer.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.3/bin/cl430" -vmsp --abi=eabi --use_hw_mpy=none --preinclude="C:/Users/Mony/workspace_v6_0/SimplexTransfer_ENDPOINT/Application/EndPoint/SimplexTransferLR09Config.h" --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.3/include" --include_path="C:/Users/Mony/workspace_v6_0/SimplexTransfer_ENDPOINT/Application" --include_path="C:/Users/Mony/workspace_v6_0/SimplexTransfer_ENDPOINT/Protocol" --include_path="C:/Users/Mony/workspace_v6_0/SimplexTransfer_ENDPOINT/Protocol/API" --include_path="C:/Users/Mony/workspace_v6_0/SimplexTransfer_ENDPOINT/Protocol/DataLink" --include_path="C:/Users/Mony/workspace_v6_0/SimplexTransfer_ENDPOINT/Protocol/DataLink/MAC" --include_path="C:/Users/Mony/workspace_v6_0/SimplexTransfer_ENDPOINT/Protocol/DataLink/PhyBridge" --include_path="C:/Users/Mony/workspace_v6_0/SimplexTransfer_ENDPOINT/Protocol/Physical" --include_path="C:/Users/Mony/workspace_v6_0/SimplexTransfer_ENDPOINT/Protocol/Physical/Driver" --include_path="C:/Users/Mony/workspace_v6_0/SimplexTransfer_ENDPOINT/Protocol/Physical/Module" --include_path="C:/Users/Mony/workspace_v6_0/SimplexTransfer_ENDPOINT/Protocol/Physical/Module/A110LR09" --include_path="C:/Users/Mony/workspace_v6_0/SimplexTransfer_ENDPOINT/Protocol/Physical/PhyBridge" --advice:power=all -g --define=__MSP430G2553__ --diag_warning=225 --display_error_number --diag_wrap=off --printf_support=minimal --preproc_with_compile --preproc_dependency="Application/EndPoint/SimplexTransfer.pp" --obj_directory="Application/EndPoint" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


