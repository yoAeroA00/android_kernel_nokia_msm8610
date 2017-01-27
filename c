mv .config config
make clean
make mrproper
mv config .config
rm ~/Nokia-X2/android_kernel_nokia_msm8610/arch/arm/boot/dtb
rm ~/Nokia-X2/android_kernel_nokia_msm8610/arch/arm/crypto/aesbs-core.S
rm ~/Nokia-X2/android_kernel_nokia_msm8610/arch/arm/crypto/sha256-core.S
