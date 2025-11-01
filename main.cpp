#include <Geode/modify/FLAlertLayer.hpp>

using namespace geode::prelude;

class $modify(CAAlertLayer, FLAlertLayer) {
    struct Fields {
        bool m_overrideBack = false;
    };

    static void onModify(ModifyBase<ModifyDerive<CAAlertLayer, FLAlertLayer>>& self) {
        (void)self.setHookPriority("FLAlertLayer::keyBackClicked", Priority::Replace);
    }

    bool init(FLAlertLayerProtocol* p0, const char* p1, gd::string p2, const char* p3, const char* p4, float p5, bool p6, float p7, float p8) {
        if (!FLAlertLayer::init(p0, p1, p2, p3, p4, p5, p6, p7, p8)) return false;

        auto background = m_mainLayer->getChildByID("background");
        if (!background) return true;

        auto& contentSize = background->getContentSize();

        m_fields->m_overrideBack = true;

        auto closeSprite = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
        closeSprite->setScale(0.8f);

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
        if (m_fields->m_overrideBack) onCloseButton(nullptr);
        else FLAlertLayer::keyBackClicked();
    }
};

#ifdef GEODE_IS_MACOS
uintptr_t keyBackClickedAddress = base::get() + GEODE_ARM_MAC(0x3f64cc) GEODE_INTEL_MAC(0x4890f0);

void keyBackClickedHook(uintptr_t self) {
    auto layer = reinterpret_cast<CAAlertLayer*>(self - 0x150);
    if (layer->m_fields->m_overrideBack) layer->onCloseButton(nullptr);
    else reinterpret_cast<void(*)(uintptr_t)>(keyBackClickedAddress)(self);
}

$execute {
    auto hook = Hook::create(
        reinterpret_cast<void*>(keyBackClickedAddress),
        &keyBackClickedHook,
        "FLAlertLayer::CCKeypadDelegate::keyBackClicked",
        tulip::hook::TulipConvention::Default
    );
    hook->setPriority(Priority::Replace);
    if (auto res = Mod::get()->claimHook(hook); res.isErr()) {
        log::error("Failed to hook FLAlertLayer::CCKeypadDelegate::keyBackClicked: {}", std::move(res).unwrapErr());
    }
}
#endif
