// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/intents/web_intent_inline_disposition_delegate.h"
#include "chrome/browser/ui/intents/web_intent_picker.h"
#include "chrome/test/base/browser_with_test_window_test.h"
#include "chrome/test/base/testing_profile.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/url_constants.h"
#include "testing/gtest/include/gtest/gtest.h"

class WebIntentPickerMock : public WebIntentPicker {
 public:
  virtual void Close() OVERRIDE {}
  virtual void SetActionString(const string16& action) OVERRIDE {}
  virtual void OnExtensionInstallSuccess(const std::string& id) OVERRIDE {}
  virtual void OnExtensionInstallFailure(const std::string& id) OVERRIDE {}
  virtual void OnInlineDispositionAutoResize(const gfx::Size& size) OVERRIDE {}
  virtual void OnPendingAsyncCompleted() OVERRIDE {}
  virtual void InvalidateDelegate() OVERRIDE {}
};

class WebIntentInlineDispositionBrowserTest
    : public BrowserWithTestWindowTest {
 public:
  virtual void SetUp() {
    BrowserWithTestWindowTest::SetUp();

    web_contents_.reset(content::WebContents::Create(
        content::WebContents::CreateParams(browser()->profile())));
    delegate_.reset(new WebIntentInlineDispositionDelegate(
        &mock_, web_contents_.get(), browser()));
  }

 protected:
  TestingProfile profile_;
  scoped_ptr<content::WebContents> web_contents_;
  scoped_ptr<WebIntentInlineDispositionDelegate> delegate_;
  WebIntentPickerMock mock_;
};

// Verifies delegate's OpenURLFromTab works. This allows navigation inside
// web intents picker.
TEST_F(WebIntentInlineDispositionBrowserTest, OpenURLFromTabTest) {
  content::WebContents* source = delegate_->OpenURLFromTab(
    web_contents_.get(),
    content::OpenURLParams(GURL(chrome::kAboutBlankURL),
    content::Referrer(),
    NEW_FOREGROUND_TAB,
    content::PAGE_TRANSITION_LINK,
    false));

  // Ensure OpenURLFromTab loaded about::blank in |web_contents_|.
  ASSERT_EQ(web_contents_.get(), source);
  EXPECT_EQ(GURL(chrome::kAboutBlankURL).spec(), source->GetURL().spec());
}
