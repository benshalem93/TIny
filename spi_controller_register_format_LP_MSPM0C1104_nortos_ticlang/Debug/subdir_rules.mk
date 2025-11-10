################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs2031/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"C:/Projects/TIny/spi_controller_register_format_LP_MSPM0C1104_nortos_ticlang" -I"C:/Projects/TIny/spi_controller_register_format_LP_MSPM0C1104_nortos_ticlang/Debug" -I"C:/ti/mspm0_sdk_2_07_00_05/source/third_party/CMSIS/Core/Include" -I"C:/ti/mspm0_sdk_2_07_00_05/source" -gdwarf-3 -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

build-2024081361: ../spi_controller_register_format.syscfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: SysConfig'
	"C:/ti/ccs2031/ccs/utils/sysconfig_1.25.0/sysconfig_cli.bat" --script "C:/Projects/TIny/spi_controller_register_format_LP_MSPM0C1104_nortos_ticlang/spi_controller_register_format.syscfg" -o "." -s "C:/ti/mspm0_sdk_2_07_00_05/.metadata/product.json" --compiler ticlang
	@echo 'Finished building: "$<"'
	@echo ' '

device_linker.cmd: build-2024081361 ../spi_controller_register_format.syscfg
device.opt: build-2024081361
device.cmd.genlibs: build-2024081361
ti_msp_dl_config.c: build-2024081361
ti_msp_dl_config.h: build-2024081361
Event.dot: build-2024081361

%.o: ./%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs2031/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"C:/Projects/TIny/spi_controller_register_format_LP_MSPM0C1104_nortos_ticlang" -I"C:/Projects/TIny/spi_controller_register_format_LP_MSPM0C1104_nortos_ticlang/Debug" -I"C:/ti/mspm0_sdk_2_07_00_05/source/third_party/CMSIS/Core/Include" -I"C:/ti/mspm0_sdk_2_07_00_05/source" -gdwarf-3 -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

startup_mspm0c110x_ticlang.o: C:/ti/mspm0_sdk_2_07_00_05/source/ti/devices/msp/m0p/startup_system_files/ticlang/startup_mspm0c110x_ticlang.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs2031/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"C:/Projects/TIny/spi_controller_register_format_LP_MSPM0C1104_nortos_ticlang" -I"C:/Projects/TIny/spi_controller_register_format_LP_MSPM0C1104_nortos_ticlang/Debug" -I"C:/ti/mspm0_sdk_2_07_00_05/source/third_party/CMSIS/Core/Include" -I"C:/ti/mspm0_sdk_2_07_00_05/source" -gdwarf-3 -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


