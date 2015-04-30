################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
Protocol/API/API.obj: ../Protocol/API/API.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.3/bin/cl430" -vmsp --abi=eabi --opt_for_speed=4 --use_hw_mpy=none --preinclude="C:/Users/Mony/workspace_v6_0/HalfDuplexTransfer/Application/Gateway/HalfDuplexTransferLR09Config.h" --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.3/include" --include_path="C:/Users/Mony/workspace_v6_0/HalfDuplexTransfer/Application" --include_path="C:/Users/Mony/workspace_v6_0/HalfDuplexTransfer/Protocol" --include_path="C:/Users/Mony/workspace_v6_0/HalfDuplexTransfer/Protocol/API" --include_path="C:/Users/Mony/workspace_v6_0/HalfDuplexTransfer/Protocol/DataLink" --include_path="C:/Users/Mony/workspace_v6_0/HalfDuplexTransfer/Protocol/DataLink/MAC" --include_path="C:/Users/Mony/workspace_v6_0/HalfDuplexTransfer/Protocol/DataLink/PhyBridge" --include_path="C:/Users/Mony/workspace_v6_0/HalfDuplexTransfer/Protocol/Physical" --include_path="C:/Users/Mony/workspace_v6_0/HalfDuplexTransfer/Protocol/Physical/Driver" --include_path="C:/Users/Mony/workspace_v6_0/HalfDuplexTransfer/Protocol/Physical/Module" --include_path="C:/Users/Mony/workspace_v6_0/HalfDuplexTransfer/Protocol/Physical/Module/A110LR09" --include_path="C:/Users/Mony/workspace_v6_0/HalfDuplexTransfer/Protocol/Physical/PhyBridge" --advice:power=all -g --define=__MSP430G2553__ --diag_warning=225 --display_error_number --diag_wrap=off --printf_support=minimal --preproc_with_compile --preproc_dependency="Protocol/API/API.pp" --obj_directory="Protocol/API" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


