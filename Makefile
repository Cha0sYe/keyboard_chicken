.PHONY: default clean prebuilt

NDKBUILT := \
  libs/arm64-v8a/chicken \
  libs/armeabi/chicken \
  libs/armeabi-v7a/chicken \
  libs/mips/chicken \
  libs/mips64/chicken \
  libs/x86/chicken \
  libs/x86_64/chicken \

default: prebuilt

clean:
	ndk-build clean
	rm -rf prebuilt

$(NDKBUILT):
	ndk-build

# It may feel a bit redundant to list everything here. However it also
# acts as a safeguard to make sure that we really are including everything
# that is supposed to be there.
prebuilt: \
  prebuilt/arm64-v8a/bin/chicken \
  prebuilt/armeabi/bin/chicken \
  prebuilt/armeabi-v7a/bin/chicken \
  prebuilt/mips/bin/chicken \
  prebuilt/mips64/bin/chicken \
  prebuilt/x86/bin/chicken \
  prebuilt/x86_64/bin/chicken \

prebuilt/%/bin/chicken: libs/%/chicken
	mkdir -p $(@D)
	cp $^ $@
