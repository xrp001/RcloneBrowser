#include "s3_tags.h"

#include "utils.h"

namespace {

struct S3RemoteConfig {
  bool ok;
  QString error;
  QString provider;
  QString region;
  QString endpoint;
  QString accessKeyId;
  QString secretAccessKey;
};

struct S3RequestResult {
  bool ok;
  QString error;
  QByteArray body;
};

QString TrimSlashes(const QString &value) {
  QString result = value.trimmed();
  while (result.startsWith('/')) {
    result.remove(0, 1);
  }
  while (result.endsWith('/')) {
    result.chop(1);
  }
  return result;
}

QString UrlEncodePathSegment(const QString &value) {
  QByteArray encoded = QUrl::toPercentEncoding(value, QByteArray(),
                                               QByteArray("~"));
  return QString::fromUtf8(encoded);
}

QString BuildCanonicalUri(const QString &bucket, const QString &key) {
  QStringList parts;
  parts << bucket;
  
  // IMPORTANT: Handle literal '%' characters properly
  // S3 object keys can contain '%' as literal character (not part of URL encoding)
  // Only skip encoding if we find VALID %XX hex encoding patterns
  // If '%' is NOT followed by two hex digits, it is a literal and MUST be encoded to %25
  for (const auto &part : key.split('/', Qt::SkipEmptyParts)) {
    bool hasOnlyValidEncoding = true;
    int i = 0;
    while (i < part.length()) {
      if (part[i] == '%') {
        // Check if this is a valid %XX encoding pattern
        if (i + 2 >= part.length() 
            || !part[i+1].isLetterOrNumber() 
            || !part[i+2].isLetterOrNumber()) {
          // Invalid encoding pattern, this is literal % character
          hasOnlyValidEncoding = false;
          break;
        }
        i += 3;
      } else {
        i += 1;
      }
    }
    
    if (hasOnlyValidEncoding && part.contains('%')) {
      // All % characters are part of valid URL encoding, use as-is
      parts << part;
    } else {
      // Contains literal % or no encoding at all, encode entire segment properly
      parts << UrlEncodePathSegment(part);
    }
  }

  return "/" + parts.join('/');
}

QByteArray HmacSha256(const QByteArray &key, const QByteArray &data) {
  return QMessageAuthenticationCode::hash(data, key, QCryptographicHash::Sha256);
}

QByteArray HexSha256(const QByteArray &data) {
  return QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex();
}

QString GetIniValue(const QJsonObject &remote, const QString &key) {
  return remote.value(key).toString().trimmed();
}

S3RemoteConfig LoadS3RemoteConfig(const QString &remoteName) {
  S3RemoteConfig result;
  result.ok = false;

  QProcess process;
  UseRclonePassword(&process);
  process.setProgram(GetRclone());
  process.setArguments(QStringList() << "config"
                                     << "dump"
                                     << GetRcloneConf()
                                     << "--ask-password=false");
  process.start();

  if (!process.waitForFinished()) {
    result.error = "Failed to run rclone config dump.";
    return result;
  }

  QByteArray stdoutData = process.readAllStandardOutput();
  QByteArray stderrData = process.readAllStandardError();

  if (process.exitCode() != 0) {
    result.error = QString::fromUtf8(stderrData).trimmed();
    if (result.error.isEmpty()) {
      result.error = QString::fromUtf8(stdoutData).trimmed();
    }
    if (result.error.isEmpty()) {
      result.error = "rclone config dump failed.";
    }
    return result;
  }

  QJsonParseError parseError;
  QJsonDocument document = QJsonDocument::fromJson(stdoutData, &parseError);
  if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
    result.error = "Failed to parse rclone config dump output.";
    return result;
  }

  QJsonObject root = document.object();
  if (!root.contains(remoteName) || !root.value(remoteName).isObject()) {
    result.error =
        QString("Cannot find remote '%1' in rclone config dump.").arg(remoteName);
    return result;
  }

  QJsonObject remote = root.value(remoteName).toObject();
  QString type = GetIniValue(remote, "type");
  if (!type.startsWith("s3", Qt::CaseInsensitive)) {
    result.error = QString("Remote '%1' is not an S3 remote.").arg(remoteName);
    return result;
  }

  result.provider = GetIniValue(remote, "provider");
  result.region = GetIniValue(remote, "region");
  result.endpoint = GetIniValue(remote, "endpoint");
  result.accessKeyId = GetIniValue(remote, "access_key_id");
  result.secretAccessKey = GetIniValue(remote, "secret_access_key");

  if (result.accessKeyId.isEmpty() || result.secretAccessKey.isEmpty()) {
    result.error =
        "Only remotes with explicit access_key_id and secret_access_key are "
        "currently supported for object tags.";
    return result;
  }

  if (result.region.isEmpty()) {
    result.region = "us-east-1";
  }

  if (result.endpoint.isEmpty()) {
    if (result.provider.compare("AWS", Qt::CaseInsensitive) == 0 ||
        result.provider.isEmpty()) {
      if (result.region == "us-east-1") {
        result.endpoint = "https://s3.amazonaws.com";
      } else {
        result.endpoint = QString("https://s3.%1.amazonaws.com").arg(result.region);
      }
    } else {
      result.error =
          "This S3 remote has no endpoint configured, so object tags cannot "
          "be accessed reliably.";
      return result;
    }
  }

  result.ok = true;
  return result;
}

bool SplitBucketAndKey(const QString &objectPath, QString *bucket, QString *key,
                       QString *error) {
  QString path = TrimSlashes(objectPath);
  if (path.isEmpty()) {
    *error = "Object path is empty.";
    return false;
  }

  int slash = path.indexOf('/');
  if (slash <= 0 || slash >= path.length() - 1) {
    *error = "Please select a single object inside an S3 bucket.";
    return false;
  }

  *bucket = path.left(slash);
  *key = path.mid(slash + 1);
  return true;
}

QString CanonicalizeAmzHeaders(const QMap<QString, QString> &headers) {
  QStringList lines;
  for (auto it = headers.constBegin(); it != headers.constEnd(); ++it) {
    lines << it.key().trimmed().toLower() + ":" + it.value().trimmed();
  }
  return lines.join('\n') + '\n';
}

QString SignedHeaders(const QMap<QString, QString> &headers) {
  QStringList names;
  for (auto it = headers.constBegin(); it != headers.constEnd(); ++it) {
    names << it.key().trimmed().toLower();
  }
  return names.join(';');
}

S3RequestResult SendS3TaggingRequest(const QString &method,
                                     const QString &remoteName,
                                     const QString &objectPath,
                                     const QList<S3ObjectTag> &tags) {
  S3RequestResult result;
  result.ok = false;

  S3RemoteConfig config = LoadS3RemoteConfig(remoteName);
  if (!config.ok) {
    result.error = config.error;
    return result;
  }

  QString bucket;
  QString key;
  if (!SplitBucketAndKey(objectPath, &bucket, &key, &result.error)) {
    return result;
  }

  QUrl endpoint(config.endpoint);
  if (!endpoint.isValid() || endpoint.scheme().isEmpty() || endpoint.host().isEmpty()) {
    result.error = QString("Invalid S3 endpoint: %1").arg(config.endpoint);
    return result;
  }

  QString canonicalUri = BuildCanonicalUri(bucket, key);
  QUrl url = endpoint;
  // CRITICAL: Use StrictMode to prevent QUrl from automatically decoding % characters!
  // QUrl by default decodes path components which destroys object keys that contain literal %
  url.setPath(canonicalUri, QUrl::StrictMode);
  url.setQuery("tagging", QUrl::StrictMode);

  QByteArray payload;
  if (method == "PUT") {
    QString xml =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<Tagging xmlns=\"http://s3.amazonaws.com/doc/2006-03-01/\">"
        "<TagSet>";
    for (const auto &tag : tags) {
      xml += "<Tag><Key>" + tag.key.toHtmlEscaped() + "</Key><Value>" +
             tag.value.toHtmlEscaped() + "</Value></Tag>";
    }
    xml += "</TagSet></Tagging>";
    payload = xml.toUtf8();
  }

  QString service = "s3";
  QDateTime now = QDateTime::currentDateTimeUtc();
  QString amzDate = now.toString("yyyyMMdd'T'HHmmss'Z'");
  QString dateStamp = now.toString("yyyyMMdd");
  QByteArray payloadHash = HexSha256(payload);

  QMap<QString, QString> headers;
  headers.insert("host", url.host());
  headers.insert("x-amz-content-sha256", QString::fromLatin1(payloadHash));
  headers.insert("x-amz-date", amzDate);
  if (method == "PUT") {
    headers.insert("content-type", "application/xml");
  }

  QString canonicalRequest =
      method + "\n" + canonicalUri + "\n" + "tagging=" + "\n" +
      CanonicalizeAmzHeaders(headers) + "\n" + SignedHeaders(headers) + "\n" +
      QString::fromLatin1(payloadHash);

  QString credentialScope =
      dateStamp + "/" + config.region + "/" + service + "/aws4_request";
  QByteArray stringToSign =
      "AWS4-HMAC-SHA256\n" + amzDate.toUtf8() + "\n" +
      credentialScope.toUtf8() + "\n" + HexSha256(canonicalRequest.toUtf8());

  QByteArray kDate =
      HmacSha256(("AWS4" + config.secretAccessKey).toUtf8(), dateStamp.toUtf8());
  QByteArray kRegion = HmacSha256(kDate, config.region.toUtf8());
  QByteArray kService = HmacSha256(kRegion, service.toUtf8());
  QByteArray kSigning = HmacSha256(kService, "aws4_request");
  QByteArray signature = HmacSha256(kSigning, stringToSign).toHex();

  QString authorization =
      QString("AWS4-HMAC-SHA256 Credential=%1/%2, SignedHeaders=%3, Signature=%4")
          .arg(config.accessKeyId)
          .arg(credentialScope)
          .arg(SignedHeaders(headers))
          .arg(QString::fromLatin1(signature));

  QNetworkRequest request(url);
  request.setRawHeader("Host", url.host().toUtf8());
  request.setRawHeader("x-amz-date", amzDate.toUtf8());
  request.setRawHeader("x-amz-content-sha256", payloadHash);
  request.setRawHeader("Authorization", authorization.toUtf8());
  if (method == "PUT") {
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/xml");
  }

  QNetworkAccessManager manager;
  QNetworkReply *reply = nullptr;
  if (method == "GET") {
    reply = manager.get(request);
  } else {
    reply = manager.put(request, payload);
  }

  QEventLoop loop;
  QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
  loop.exec();

  result.body = reply->readAll();
  int statusCode =
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  if (reply->error() != QNetworkReply::NoError ||
      statusCode < 200 || statusCode >= 300) {
    QString error = reply->errorString().trimmed();
    if (!result.body.trimmed().isEmpty()) {
      error += "\n" + QString::fromUtf8(result.body).trimmed();
    }
    if (error.trimmed().isEmpty()) {
      error = QString("S3 request failed with HTTP status %1.").arg(statusCode);
    }
    result.error = error.trimmed();
    reply->deleteLater();
    return result;
  }

  reply->deleteLater();
  result.ok = true;
  return result;
}

QList<S3ObjectTag> ParseTagsXml(const QByteArray &xml, QString *error) {
  QList<S3ObjectTag> tags;
  QXmlStreamReader reader(xml);

  QString currentKey;
  QString currentValue;

  while (!reader.atEnd()) {
    reader.readNext();

    if (reader.isStartElement()) {
      if (reader.name() == "Tag") {
        currentKey.clear();
        currentValue.clear();
      } else if (reader.name() == "Key") {
        currentKey = reader.readElementText();
      } else if (reader.name() == "Value") {
        currentValue = reader.readElementText();
      }
    } else if (reader.isEndElement() && reader.name() == "Tag") {
      S3ObjectTag tag;
      tag.key = currentKey;
      tag.value = currentValue;
      tags << tag;
    }
  }

  if (reader.hasError()) {
    *error = reader.errorString();
    return QList<S3ObjectTag>();
  }

  return tags;
}

} // namespace

QString FormatS3ObjectTags(const QList<S3ObjectTag> &tags) {
  QStringList lines;
  for (const auto &tag : tags) {
    lines << tag.key + ":" + tag.value;
  }
  return lines.join('\n');
}

S3ObjectTagsResult ParseS3ObjectTagsInput(const QString &text) {
  S3ObjectTagsResult result;
  result.ok = false;

  QList<S3ObjectTag> tags;
  for (const auto &rawLine : text.split('\n')) {
    QString line = rawLine.trimmed();
    if (line.isEmpty()) {
      continue;
    }

    int separator = line.indexOf(':');
    if (separator <= 0) {
      separator = line.indexOf('=');
    }
    if (separator <= 0 || separator >= line.length() - 1) {
      result.error =
          QString("Invalid tag line: %1\nUse key:value or key=value.")
              .arg(line);
      return result;
    }

    S3ObjectTag tag;
    tag.key = line.left(separator).trimmed();
    tag.value = line.mid(separator + 1).trimmed();
    if (tag.key.isEmpty()) {
      result.error = QString("Invalid empty tag key in line: %1").arg(line);
      return result;
    }
    tags << tag;
  }

  result.ok = true;
  result.tags = tags;
  return result;
}

S3ObjectTagsResult GetS3ObjectTags(const QString &remoteName,
                                   const QString &objectPath) {
  S3ObjectTagsResult result;
  result.ok = false;

  S3RequestResult requestResult =
      SendS3TaggingRequest("GET", remoteName, objectPath, QList<S3ObjectTag>());
  if (!requestResult.ok) {
    result.error = requestResult.error;
    return result;
  }

  QString error;
  result.tags = ParseTagsXml(requestResult.body, &error);
  if (!error.isEmpty()) {
    result.error = error;
    return result;
  }

  result.ok = true;
  return result;
}

S3ObjectTagsResult SetS3ObjectTags(const QString &remoteName,
                                   const QString &objectPath,
                                   const QList<S3ObjectTag> &tags) {
  S3ObjectTagsResult result;
  result.ok = false;

  S3RequestResult requestResult =
      SendS3TaggingRequest("PUT", remoteName, objectPath, tags);
  if (!requestResult.ok) {
    result.error = requestResult.error;
    return result;
  }

  result.ok = true;
  result.tags = tags;
  return result;
}