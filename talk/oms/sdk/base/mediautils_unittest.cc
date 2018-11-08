/*
 * Intel License
 */
#include "talk/oms/sdk/base/mediautils.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/gmock/include/gmock/gmock.h"
namespace oms {
namespace base {
TEST(MediaUtilsTest, CorrectResolutionNameHd720p){
  const Resolution r(1280, 720);
  EXPECT_EQ(MediaUtils::GetResolutionName(r),"hd720p");
}
}
}
