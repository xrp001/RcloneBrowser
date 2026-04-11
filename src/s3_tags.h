#pragma once

#include "pch.h"

struct S3ObjectTag {
  QString key;
  QString value;
};

struct S3ObjectTagsResult {
  bool ok;
  QString error;
  QList<S3ObjectTag> tags;
};

S3ObjectTagsResult GetS3ObjectTags(const QString &remoteName,
                                   const QString &objectPath);
S3ObjectTagsResult SetS3ObjectTags(const QString &remoteName,
                                   const QString &objectPath,
                                   const QList<S3ObjectTag> &tags);
QString FormatS3ObjectTags(const QList<S3ObjectTag> &tags);
S3ObjectTagsResult ParseS3ObjectTagsInput(const QString &text);