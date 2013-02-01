NACLPORTS_STAMP=out/naclports.stamp
COCOS2DX_STAMP=out/cocos2dx.stamp

all: $(NACLPORTS_STAMP) $(COCOS2DX_STAMP) game

naclports $(NACLPORTS_STAMP): build/build-naclports.sh
	build/build-naclports.sh
	touch $(NACLPORTS_STAMP)

cocos2dx $(COCOS2DX_STAMP): build/build-cocos2dx.sh
	build/build-cocos2dx.sh
	touch $(COCOS2DX_STAMP)

include game.mk

clean::
	rm -f $(NACLPORTS_STAMP) $(COCOS2DX_STAMP)

.PHONY: naclports cocos2dx
