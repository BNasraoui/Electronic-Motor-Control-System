################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
drivers/GUI_LogGraph.obj: C:/Users/jesse/Dropbox/Sem1_2021/Embedded/CCS_Workspace/EGH456_GROUP6/resources/GUI_LogGraph.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1020/ccs/tools/compiler/ti-cgt-arm_20.2.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/ti/tivaware_c_series_2_1_4_178/examples/boards/ek-tm4c1294xl-boostxl-kentec-s1" --include_path="C:/ti/tivaware_c_series_2_1_4_178" --include_path="C:/Users/jesse/Dropbox/Sem1_2021/Embedded/CCS_Workspace/EGH456_GROUP6/src" --include_path="C:/Users/jesse/Dropbox/Sem1_2021/Embedded/CCS_Workspace/EGH456_GROUP6/src" --include_path="C:/ti/tirtos_tivac_2_16_00_08/products/TivaWare_C_Series-2.1.1.71b" --include_path="C:/ti/tirtos_tivac_2_16_00_08/products/bios_6_45_01_29/packages/ti/sysbios/posix" --include_path="C:/Users/jesse/Dropbox/Sem1_2021/Embedded/CCS_Workspace/EGH456_GROUP6/src/drivers" --include_path="C:/Users/jesse/Dropbox/Sem1_2021/Embedded/CCS_Workspace/EGH456_GROUP6/resources" --include_path="C:/ti/ccs1020/ccs/tools/compiler/ti-cgt-arm_20.2.2.LTS/include" --define=ccs="ccs" --define=PART_TM4C1294NCPDT --define=ccs --define=TIVAWARE -g --gcc --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="drivers/$(basename $(<F)).d_raw" --obj_directory="drivers" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

drivers/GUI_XYGraph.obj: C:/Users/jesse/Dropbox/Sem1_2021/Embedded/CCS_Workspace/EGH456_GROUP6/resources/GUI_XYGraph.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1020/ccs/tools/compiler/ti-cgt-arm_20.2.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/ti/tivaware_c_series_2_1_4_178/examples/boards/ek-tm4c1294xl-boostxl-kentec-s1" --include_path="C:/ti/tivaware_c_series_2_1_4_178" --include_path="C:/Users/jesse/Dropbox/Sem1_2021/Embedded/CCS_Workspace/EGH456_GROUP6/src" --include_path="C:/Users/jesse/Dropbox/Sem1_2021/Embedded/CCS_Workspace/EGH456_GROUP6/src" --include_path="C:/ti/tirtos_tivac_2_16_00_08/products/TivaWare_C_Series-2.1.1.71b" --include_path="C:/ti/tirtos_tivac_2_16_00_08/products/bios_6_45_01_29/packages/ti/sysbios/posix" --include_path="C:/Users/jesse/Dropbox/Sem1_2021/Embedded/CCS_Workspace/EGH456_GROUP6/src/drivers" --include_path="C:/Users/jesse/Dropbox/Sem1_2021/Embedded/CCS_Workspace/EGH456_GROUP6/resources" --include_path="C:/ti/ccs1020/ccs/tools/compiler/ti-cgt-arm_20.2.2.LTS/include" --define=ccs="ccs" --define=PART_TM4C1294NCPDT --define=ccs --define=TIVAWARE -g --gcc --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="drivers/$(basename $(<F)).d_raw" --obj_directory="drivers" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

drivers/GUI_graph.obj: C:/Users/jesse/Dropbox/Sem1_2021/Embedded/CCS_Workspace/EGH456_GROUP6/resources/GUI_graph.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1020/ccs/tools/compiler/ti-cgt-arm_20.2.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/ti/tivaware_c_series_2_1_4_178/examples/boards/ek-tm4c1294xl-boostxl-kentec-s1" --include_path="C:/ti/tivaware_c_series_2_1_4_178" --include_path="C:/Users/jesse/Dropbox/Sem1_2021/Embedded/CCS_Workspace/EGH456_GROUP6/src" --include_path="C:/Users/jesse/Dropbox/Sem1_2021/Embedded/CCS_Workspace/EGH456_GROUP6/src" --include_path="C:/ti/tirtos_tivac_2_16_00_08/products/TivaWare_C_Series-2.1.1.71b" --include_path="C:/ti/tirtos_tivac_2_16_00_08/products/bios_6_45_01_29/packages/ti/sysbios/posix" --include_path="C:/Users/jesse/Dropbox/Sem1_2021/Embedded/CCS_Workspace/EGH456_GROUP6/src/drivers" --include_path="C:/Users/jesse/Dropbox/Sem1_2021/Embedded/CCS_Workspace/EGH456_GROUP6/resources" --include_path="C:/ti/ccs1020/ccs/tools/compiler/ti-cgt-arm_20.2.2.LTS/include" --define=ccs="ccs" --define=PART_TM4C1294NCPDT --define=ccs --define=TIVAWARE -g --gcc --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="drivers/$(basename $(<F)).d_raw" --obj_directory="drivers" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

drivers/Kentec320x240x16_ssd2119_spi.obj: C:/Users/jesse/Dropbox/Sem1_2021/Embedded/CCS_Workspace/EGH456_GROUP6/resources/Kentec320x240x16_ssd2119_spi.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1020/ccs/tools/compiler/ti-cgt-arm_20.2.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/ti/tivaware_c_series_2_1_4_178/examples/boards/ek-tm4c1294xl-boostxl-kentec-s1" --include_path="C:/ti/tivaware_c_series_2_1_4_178" --include_path="C:/Users/jesse/Dropbox/Sem1_2021/Embedded/CCS_Workspace/EGH456_GROUP6/src" --include_path="C:/Users/jesse/Dropbox/Sem1_2021/Embedded/CCS_Workspace/EGH456_GROUP6/src" --include_path="C:/ti/tirtos_tivac_2_16_00_08/products/TivaWare_C_Series-2.1.1.71b" --include_path="C:/ti/tirtos_tivac_2_16_00_08/products/bios_6_45_01_29/packages/ti/sysbios/posix" --include_path="C:/Users/jesse/Dropbox/Sem1_2021/Embedded/CCS_Workspace/EGH456_GROUP6/src/drivers" --include_path="C:/Users/jesse/Dropbox/Sem1_2021/Embedded/CCS_Workspace/EGH456_GROUP6/resources" --include_path="C:/ti/ccs1020/ccs/tools/compiler/ti-cgt-arm_20.2.2.LTS/include" --define=ccs="ccs" --define=PART_TM4C1294NCPDT --define=ccs --define=TIVAWARE -g --gcc --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="drivers/$(basename $(<F)).d_raw" --obj_directory="drivers" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

drivers/frame.obj: C:/Users/jesse/Dropbox/Sem1_2021/Embedded/CCS_Workspace/EGH456_GROUP6/resources/frame.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1020/ccs/tools/compiler/ti-cgt-arm_20.2.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/ti/tivaware_c_series_2_1_4_178/examples/boards/ek-tm4c1294xl-boostxl-kentec-s1" --include_path="C:/ti/tivaware_c_series_2_1_4_178" --include_path="C:/Users/jesse/Dropbox/Sem1_2021/Embedded/CCS_Workspace/EGH456_GROUP6/src" --include_path="C:/Users/jesse/Dropbox/Sem1_2021/Embedded/CCS_Workspace/EGH456_GROUP6/src" --include_path="C:/ti/tirtos_tivac_2_16_00_08/products/TivaWare_C_Series-2.1.1.71b" --include_path="C:/ti/tirtos_tivac_2_16_00_08/products/bios_6_45_01_29/packages/ti/sysbios/posix" --include_path="C:/Users/jesse/Dropbox/Sem1_2021/Embedded/CCS_Workspace/EGH456_GROUP6/src/drivers" --include_path="C:/Users/jesse/Dropbox/Sem1_2021/Embedded/CCS_Workspace/EGH456_GROUP6/resources" --include_path="C:/ti/ccs1020/ccs/tools/compiler/ti-cgt-arm_20.2.2.LTS/include" --define=ccs="ccs" --define=PART_TM4C1294NCPDT --define=ccs --define=TIVAWARE -g --gcc --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="drivers/$(basename $(<F)).d_raw" --obj_directory="drivers" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

drivers/sensors.obj: C:/Users/jesse/Dropbox/Sem1_2021/Embedded/CCS_Workspace/EGH456_GROUP6/resources/sensors.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1020/ccs/tools/compiler/ti-cgt-arm_20.2.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/ti/tivaware_c_series_2_1_4_178/examples/boards/ek-tm4c1294xl-boostxl-kentec-s1" --include_path="C:/ti/tivaware_c_series_2_1_4_178" --include_path="C:/Users/jesse/Dropbox/Sem1_2021/Embedded/CCS_Workspace/EGH456_GROUP6/src" --include_path="C:/Users/jesse/Dropbox/Sem1_2021/Embedded/CCS_Workspace/EGH456_GROUP6/src" --include_path="C:/ti/tirtos_tivac_2_16_00_08/products/TivaWare_C_Series-2.1.1.71b" --include_path="C:/ti/tirtos_tivac_2_16_00_08/products/bios_6_45_01_29/packages/ti/sysbios/posix" --include_path="C:/Users/jesse/Dropbox/Sem1_2021/Embedded/CCS_Workspace/EGH456_GROUP6/src/drivers" --include_path="C:/Users/jesse/Dropbox/Sem1_2021/Embedded/CCS_Workspace/EGH456_GROUP6/resources" --include_path="C:/ti/ccs1020/ccs/tools/compiler/ti-cgt-arm_20.2.2.LTS/include" --define=ccs="ccs" --define=PART_TM4C1294NCPDT --define=ccs --define=TIVAWARE -g --gcc --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="drivers/$(basename $(<F)).d_raw" --obj_directory="drivers" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

drivers/touch.obj: C:/Users/jesse/Dropbox/Sem1_2021/Embedded/CCS_Workspace/EGH456_GROUP6/resources/touch.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1020/ccs/tools/compiler/ti-cgt-arm_20.2.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/ti/tivaware_c_series_2_1_4_178/examples/boards/ek-tm4c1294xl-boostxl-kentec-s1" --include_path="C:/ti/tivaware_c_series_2_1_4_178" --include_path="C:/Users/jesse/Dropbox/Sem1_2021/Embedded/CCS_Workspace/EGH456_GROUP6/src" --include_path="C:/Users/jesse/Dropbox/Sem1_2021/Embedded/CCS_Workspace/EGH456_GROUP6/src" --include_path="C:/ti/tirtos_tivac_2_16_00_08/products/TivaWare_C_Series-2.1.1.71b" --include_path="C:/ti/tirtos_tivac_2_16_00_08/products/bios_6_45_01_29/packages/ti/sysbios/posix" --include_path="C:/Users/jesse/Dropbox/Sem1_2021/Embedded/CCS_Workspace/EGH456_GROUP6/src/drivers" --include_path="C:/Users/jesse/Dropbox/Sem1_2021/Embedded/CCS_Workspace/EGH456_GROUP6/resources" --include_path="C:/ti/ccs1020/ccs/tools/compiler/ti-cgt-arm_20.2.2.LTS/include" --define=ccs="ccs" --define=PART_TM4C1294NCPDT --define=ccs --define=TIVAWARE -g --gcc --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="drivers/$(basename $(<F)).d_raw" --obj_directory="drivers" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


