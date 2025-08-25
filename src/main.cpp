#include <Geode/modify/FLAlertLayer.hpp>

using namespace geode::prelude;

class $modify(CAAlertLayer, FLAlertLayer) {
    static void onModify(ModifyBase<ModifyDerive<CAAlertLayer, FLAlertLayer>>& self) {
        (void)self.setHookPriority("FLAlertLayer::keyBackClicked", Priority::Replace);
    }

    bool init(
        FLAlertLayerProtocol* delegate, const char* title, gd::string desc, const char* btn1, const char* btn2,
        float width, bool scroll, float unused, float textScale
    ) {
        if (!FLAlertLayer::init(delegate, title, desc, btn1, btn2, width, scroll, unused, textScale)) return false;

        auto closeSprite = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
        closeSprite->setScale(0.8f);

        auto& contentSize = m_mainLayer->getChildByID("background")->getContentSize();

        auto closeButton = CCMenuItemSpriteExtra::create(closeSprite, this, menu_selector(CAAlertLayer::onCloseButton));
        closeButton->setPosition(m_buttonMenu->convertToNodeSpace(
            CCDirector::get()->getWinSize() / 2.0f + CCSize { -contentSize.width / 2.0f + 5.0f, contentSize.height / 2.0f - 5.0f }));
        closeButton->setID("close-button"_spr);
        m_buttonMenu->addChild(closeButton);

        return true;
    }

    void onCloseButton(CCObject*) {
        setKeypadEnabled(false);
        setKeyboardEnabled(false);
        CCTouchDispatcher::get()->unregisterForcePrio(this);
        removeFromParentAndCleanup(true);
    }

    void keyBackClicked() {
        onCloseButton(nullptr);
    }
};

#ifdef GEODE_IS_MACOS
void keyBackClickedHook(uintptr_t self) {
    reinterpret_cast<CAAlertLayer*>(self - sizeof(CCNode) - sizeof(CCAccelerometerDelegate) - sizeof(CCTouchDelegate))->onCloseButton(nullptr);
}

$execute {
    auto hook = Hook::create(
        reinterpret_cast<void*>(base::get() + GEODE_ARM_MAC(0x3f64cc) GEODE_INTEL_MAC(0x4890f0)),
        &keyBackClickedHook,
        "FLAlertLayer::CCKeypadDelegate::keyBackClicked",
        tulip::hook::TulipConvention::Thiscall
    );
    hook->setPriority(Priority::Replace);
    Mod::get()->claimHook(hook).inspectErr([](const std::string& err) {
        log::error("Failed to hook FLAlertLayer::CCKeypadDelegate::keyBackClicked: {}", err);
    });
}
#endif
