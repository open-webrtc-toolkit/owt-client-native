#include "TestSdk.h"

QStringList CTestSdk::s_sdkCaseList = {
    "testJoin_multipleClients_checkEventsAndInfo",
	"testJoin_withIncorrectThenCorrectServer_shouldSucceed",
	"testJoin_withIncorrectThenCorrectRoomId_shouldSucceed",
	"testJoin_withEmptyToken_shouldFail",
	"testJoin_withIncorrectContentedToken_shouldFail",
	"testJoin_twoClientsWithSameToken_shouldFailAt2nd",
	"testJoin_thenLeaveThenJoinWithSameToken_shouldFail",
	"testJoin_thenLeaveThenJoinWithDifferentToken_shouldSucceed",
	"testJoin_withEmptyRole_shouldFail",
	"testJoin_withUnsupportedRole_shouldFail",
	"testJoin_thenLeaveThenJoinWithDifferentRole_shouldSucceed",
	"testJoin_withEmptyUserName_shouldFail",
	"testJoin_withSpecialCharacterUserName_shouldSucceed",
	"testJoin_withChineseUserName_shouldSucceed",
	"testJoin_twiceWithoutWaitCallBack_shouldSucceedOnce",
	"testGetStats_publication_afterPublicationStop_shouldFail",
	"testGetStats_subscription_afterSubscriptionStop_shouldFail",
	"testGetStats_publication_afterLeave_shouldFail",
	"testGetStats_subscription_afterLeave_shouldFail",
	"testGetStats_subscription_afterRemoteStreamEnded_shouldFail"
};
