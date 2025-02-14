/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_quota_originorpatternstring_h__
#define mozilla_dom_quota_originorpatternstring_h__

#include "mozilla/dom/quota/QuotaCommon.h"

#include "mozilla/BasePrincipal.h"

BEGIN_QUOTA_NAMESPACE

class OriginScope
{
public:
  enum Type
  {
    eOrigin,
    ePattern,
    eNull
  };

private:
  struct OriginAndAttributes
  {
    nsCString mOrigin;
    PrincipalOriginAttributes mAttributes;

    OriginAndAttributes(const OriginAndAttributes& aOther)
      : mOrigin(aOther.mOrigin)
      , mAttributes(aOther.mAttributes)
    {
      MOZ_COUNT_CTOR(OriginAndAttributes);
    }

    explicit OriginAndAttributes(const nsACString& aOrigin)
      : mOrigin(aOrigin)
    {
      nsCString originNoSuffix;
      MOZ_ALWAYS_TRUE(mAttributes.PopulateFromOrigin(aOrigin, originNoSuffix));

      MOZ_COUNT_CTOR(OriginAndAttributes);
    }

    ~OriginAndAttributes()
    {
      MOZ_COUNT_DTOR(OriginAndAttributes);
    }
  };

  union {
    // eOrigin
    OriginAndAttributes* mOriginAndAttributes;

    // ePattern
    mozilla::OriginAttributesPattern* mPattern;

    // eNull
    void* mDummy;
  };

  Type mType;

public:
  static OriginScope
  FromOrigin(const nsACString& aOrigin)
  {
    return OriginScope(aOrigin);
  }

  static OriginScope
  FromPattern(const mozilla::OriginAttributesPattern& aPattern)
  {
    return OriginScope(aPattern);
  }

  static OriginScope
  FromJSONPattern(const nsAString& aJSONPattern)
  {
    return OriginScope(aJSONPattern);
  }

  static OriginScope
  FromNull()
  {
    return OriginScope();
  }

  OriginScope(const OriginScope& aOther)
  {
    if (aOther.IsOrigin()) {
      mOriginAndAttributes =
        new OriginAndAttributes(*aOther.mOriginAndAttributes);
    } else if (aOther.IsPattern()) {
      mPattern = new mozilla::OriginAttributesPattern(*aOther.mPattern);
    } else {
      mDummy = aOther.mDummy;
    }

    mType = aOther.mType;
  }

  ~OriginScope()
  {
    Destroy();
  }

  bool
  IsOrigin() const
  {
    return mType == eOrigin;
  }

  bool
  IsPattern() const
  {
    return mType == ePattern;
  }

  bool
  IsNull() const
  {
    return mType == eNull;
  }

  Type
  GetType() const
  {
    return mType;
  }

  void
  SetFromOrigin(const nsACString& aOrigin)
  {
    Destroy();

    mOriginAndAttributes = new OriginAndAttributes(aOrigin);

    mType = eOrigin;
  }

  void
  SetFromPattern(const mozilla::OriginAttributesPattern& aPattern)
  {
    Destroy();

    mPattern = new mozilla::OriginAttributesPattern(aPattern);

    mType = ePattern;
  }

  void
  SetFromJSONPattern(const nsAString& aJSONPattern)
  {
    Destroy();

    mPattern = new mozilla::OriginAttributesPattern();
    MOZ_ALWAYS_TRUE(mPattern->Init(aJSONPattern));

    mType = ePattern;
  }

  void
  SetFromNull()
  {
    Destroy();

    mDummy = nullptr;

    mType = eNull;
  }

  const nsACString&
  GetOrigin() const
  {
    MOZ_ASSERT(IsOrigin());
    MOZ_ASSERT(mOriginAndAttributes);

    return mOriginAndAttributes->mOrigin;
  }

  void
  SetOrigin(const nsACString& aOrigin)
  {
    MOZ_ASSERT(IsOrigin());
    MOZ_ASSERT(mOriginAndAttributes);
    mOriginAndAttributes->mOrigin = aOrigin;
  }

  const mozilla::OriginAttributes&
  GetOriginAttributes() const
  {
    MOZ_ASSERT(IsOrigin());
    MOZ_ASSERT(mOriginAndAttributes);
    return mOriginAndAttributes->mAttributes;
  }

  const mozilla::OriginAttributesPattern&
  GetPattern() const
  {
    MOZ_ASSERT(IsPattern());
    MOZ_ASSERT(mPattern);
    return *mPattern;
  }

  bool MatchesOrigin(const OriginScope& aOther) const
  {
    MOZ_ASSERT(aOther.IsOrigin());
    MOZ_ASSERT(aOther.mOriginAndAttributes);

    bool match;

    if (IsOrigin()) {
      MOZ_ASSERT(mOriginAndAttributes);
      match = mOriginAndAttributes->mOrigin.Equals(
                aOther.mOriginAndAttributes->mOrigin);
    } else if (IsPattern()) {
      MOZ_ASSERT(mPattern);
      match = mPattern->Matches(aOther.mOriginAndAttributes->mAttributes);
    } else {
      match = true;
    }

    return match;
  }

  bool MatchesPattern(const OriginScope& aOther) const
  {
    MOZ_ASSERT(aOther.IsPattern());
    MOZ_ASSERT(aOther.mPattern);

    bool match;

    if (IsOrigin()) {
      MOZ_ASSERT(mOriginAndAttributes);
      match = aOther.mPattern->Matches(mOriginAndAttributes->mAttributes);
    } else if (IsPattern()) {
      MOZ_ASSERT(mPattern);
      match = mPattern->Overlaps(*aOther.mPattern);
    } else {
      match = true;
    }

    return match;
  }

  bool Matches(const OriginScope& aOther) const
  {
    bool match;

    if (aOther.IsOrigin()) {
      match = MatchesOrigin(aOther);
    } else if (aOther.IsPattern()) {
      match = MatchesPattern(aOther);
    } else {
      match = true;
    }

    return match;
  }

  OriginScope
  Clone()
  {
    if (IsOrigin()) {
      MOZ_ASSERT(mOriginAndAttributes);
      return OriginScope(*mOriginAndAttributes);
    }

    if (IsPattern()) {
      MOZ_ASSERT(mPattern);
      return OriginScope(*mPattern);
    }

    MOZ_ASSERT(IsNull());
    return OriginScope();
  }

private:
  explicit OriginScope(const OriginAndAttributes& aOriginAndAttributes)
    : mOriginAndAttributes(new OriginAndAttributes(aOriginAndAttributes))
    , mType(eOrigin)
  { }

  explicit OriginScope(const nsACString& aOrigin)
    : mOriginAndAttributes(new OriginAndAttributes(aOrigin))
    , mType(eOrigin)
  { }

  explicit OriginScope(const mozilla::OriginAttributesPattern& aPattern)
    : mPattern(new mozilla::OriginAttributesPattern(aPattern))
    , mType(ePattern)
  { }

  explicit OriginScope(const nsAString& aJSONPattern)
    : mPattern(new mozilla::OriginAttributesPattern())
    , mType(ePattern)
  {
    MOZ_ALWAYS_TRUE(mPattern->Init(aJSONPattern));
  }

  OriginScope()
    : mDummy(nullptr)
    , mType(eNull)
  { }

  void
  Destroy()
  {
    if (IsOrigin()) {
      MOZ_ASSERT(mOriginAndAttributes);
      delete mOriginAndAttributes;
      mOriginAndAttributes = nullptr;
    } else if (IsPattern()) {
      MOZ_ASSERT(mPattern);
      delete mPattern;
      mPattern = nullptr;
    }
  }

  bool
  operator==(const OriginScope& aOther) = delete;
};

END_QUOTA_NAMESPACE

#endif // mozilla_dom_quota_originorpatternstring_h__
