################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/board_link.c \
../Core/Src/fat_reader.c \
../Core/Src/game_app.c \
../Core/Src/gameplay.c \
../Core/Src/ili9341.c \
../Core/Src/level1_scene.c \
../Core/Src/level2_scene.c \
../Core/Src/level3_scene.c \
../Core/Src/level_images.c \
../Core/Src/main.c \
../Core/Src/pause_screen.c \
../Core/Src/player_sprites.c \
../Core/Src/puddle_sprites.c \
../Core/Src/scene_cache.c \
../Core/Src/sd_card.c \
../Core/Src/splash_images.c \
../Core/Src/status_text.c \
../Core/Src/stm32f4xx_hal_msp.c \
../Core/Src/stm32f4xx_it.c \
../Core/Src/storage.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f4xx.c \
../Core/Src/victory_screen.c 

OBJS += \
./Core/Src/board_link.o \
./Core/Src/fat_reader.o \
./Core/Src/game_app.o \
./Core/Src/gameplay.o \
./Core/Src/ili9341.o \
./Core/Src/level1_scene.o \
./Core/Src/level2_scene.o \
./Core/Src/level3_scene.o \
./Core/Src/level_images.o \
./Core/Src/main.o \
./Core/Src/pause_screen.o \
./Core/Src/player_sprites.o \
./Core/Src/puddle_sprites.o \
./Core/Src/scene_cache.o \
./Core/Src/sd_card.o \
./Core/Src/splash_images.o \
./Core/Src/status_text.o \
./Core/Src/stm32f4xx_hal_msp.o \
./Core/Src/stm32f4xx_it.o \
./Core/Src/storage.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f4xx.o \
./Core/Src/victory_screen.o 

C_DEPS += \
./Core/Src/board_link.d \
./Core/Src/fat_reader.d \
./Core/Src/game_app.d \
./Core/Src/gameplay.d \
./Core/Src/ili9341.d \
./Core/Src/level1_scene.d \
./Core/Src/level2_scene.d \
./Core/Src/level3_scene.d \
./Core/Src/level_images.d \
./Core/Src/main.d \
./Core/Src/pause_screen.d \
./Core/Src/player_sprites.d \
./Core/Src/puddle_sprites.d \
./Core/Src/scene_cache.d \
./Core/Src/sd_card.d \
./Core/Src/splash_images.d \
./Core/Src/status_text.d \
./Core/Src/stm32f4xx_hal_msp.d \
./Core/Src/stm32f4xx_it.d \
./Core/Src/storage.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f4xx.d \
./Core/Src/victory_screen.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/board_link.cyclo ./Core/Src/board_link.d ./Core/Src/board_link.o ./Core/Src/board_link.su ./Core/Src/fat_reader.cyclo ./Core/Src/fat_reader.d ./Core/Src/fat_reader.o ./Core/Src/fat_reader.su ./Core/Src/game_app.cyclo ./Core/Src/game_app.d ./Core/Src/game_app.o ./Core/Src/game_app.su ./Core/Src/gameplay.cyclo ./Core/Src/gameplay.d ./Core/Src/gameplay.o ./Core/Src/gameplay.su ./Core/Src/ili9341.cyclo ./Core/Src/ili9341.d ./Core/Src/ili9341.o ./Core/Src/ili9341.su ./Core/Src/level1_scene.cyclo ./Core/Src/level1_scene.d ./Core/Src/level1_scene.o ./Core/Src/level1_scene.su ./Core/Src/level2_scene.cyclo ./Core/Src/level2_scene.d ./Core/Src/level2_scene.o ./Core/Src/level2_scene.su ./Core/Src/level3_scene.cyclo ./Core/Src/level3_scene.d ./Core/Src/level3_scene.o ./Core/Src/level3_scene.su ./Core/Src/level_images.cyclo ./Core/Src/level_images.d ./Core/Src/level_images.o ./Core/Src/level_images.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/pause_screen.cyclo ./Core/Src/pause_screen.d ./Core/Src/pause_screen.o ./Core/Src/pause_screen.su ./Core/Src/player_sprites.cyclo ./Core/Src/player_sprites.d ./Core/Src/player_sprites.o ./Core/Src/player_sprites.su ./Core/Src/puddle_sprites.cyclo ./Core/Src/puddle_sprites.d ./Core/Src/puddle_sprites.o ./Core/Src/puddle_sprites.su ./Core/Src/scene_cache.cyclo ./Core/Src/scene_cache.d ./Core/Src/scene_cache.o ./Core/Src/scene_cache.su ./Core/Src/sd_card.cyclo ./Core/Src/sd_card.d ./Core/Src/sd_card.o ./Core/Src/sd_card.su ./Core/Src/splash_images.cyclo ./Core/Src/splash_images.d ./Core/Src/splash_images.o ./Core/Src/splash_images.su ./Core/Src/status_text.cyclo ./Core/Src/status_text.d ./Core/Src/status_text.o ./Core/Src/status_text.su ./Core/Src/stm32f4xx_hal_msp.cyclo ./Core/Src/stm32f4xx_hal_msp.d ./Core/Src/stm32f4xx_hal_msp.o ./Core/Src/stm32f4xx_hal_msp.su ./Core/Src/stm32f4xx_it.cyclo ./Core/Src/stm32f4xx_it.d ./Core/Src/stm32f4xx_it.o ./Core/Src/stm32f4xx_it.su ./Core/Src/storage.cyclo ./Core/Src/storage.d ./Core/Src/storage.o ./Core/Src/storage.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f4xx.cyclo ./Core/Src/system_stm32f4xx.d ./Core/Src/system_stm32f4xx.o ./Core/Src/system_stm32f4xx.su ./Core/Src/victory_screen.cyclo ./Core/Src/victory_screen.d ./Core/Src/victory_screen.o ./Core/Src/victory_screen.su

.PHONY: clean-Core-2f-Src

