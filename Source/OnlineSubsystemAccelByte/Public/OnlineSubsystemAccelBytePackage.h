// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

// #NOTE: pragma once intentionally removed as unity builds can fail to compile with it

#undef PACKAGE_SCOPE
#if defined(ACCELBYTE_TEST_OVERRIDE_PACKAGE) || defined(ONLINESUBSYSTEMACCELBYTE_PACKAGE)
#define PACKAGE_SCOPE public
#else
#define PACKAGE_SCOPE protected
#endif // defined(ACCELBYTE_TEST_OVERRIDE_PACKAGE) || defined(ONLINESUBSYSTEMACCELBYTE_PACKAGE)
