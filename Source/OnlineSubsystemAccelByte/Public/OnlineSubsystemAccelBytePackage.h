// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#undef PACKAGE_SCOPE
#ifdef ONLINESUBSYSTEMACCELBYTE_PACKAGE
#define PACKAGE_SCOPE public
#else
#define PACKAGE_SCOPE protected
#endif

#undef OVERRIDE_PACKAGE_SCOPE
#if defined(ACCELBYTE_TEST_OVERRIDE_PACKAGE) || defined(ONLINESUBSYSTEMACCELBYTE_PACKAGE)
#define OVERRIDE_PACKAGE_SCOPE public
#else
#define OVERRIDE_PACKAGE_SCOPE protected
#endif

