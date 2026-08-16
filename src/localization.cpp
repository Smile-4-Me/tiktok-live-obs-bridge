// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 TikTok Live OBS Bridge Contributors

#include "localization.hpp"
#include "plugin_paths.hpp"

#include <windows.h>

#include <QDir>
#include <QHash>
#include <QSettings>

namespace {

QHash<QString, QString> translations;

QString module_locale_directory()
{
	const QDir directory(module_installation_directory());
	const QString standard_directory = directory.filePath(
		QStringLiteral("data/obs-plugins/tiktok-live-obs-bridge/locale"));
	if (QDir(standard_directory).exists())
		return standard_directory;

	// Older global installations stored locale files next to their own bin folder.
	return directory.filePath(QStringLiteral("data/locale"));
}

} // namespace

QString text(const char *key)
{
	return translations.value(QString::fromUtf8(key), QString::fromUtf8(key));
}

QString translated_or(const char *key, const QString &fallback)
{
	const QString value = text(key);
	return value == QString::fromUtf8(key) ? fallback : value;
}

QString obs_language()
{
	using LocaleStringFunction = const char *(*)(const char *);
	const HMODULE frontend = GetModuleHandleW(L"obs-frontend-api.dll");
	const auto get_locale_string = frontend ? reinterpret_cast<LocaleStringFunction>(
		GetProcAddress(frontend, "obs_frontend_get_locale_string")) : nullptr;
	if (!get_locale_string)
		return {};

	// OBS does not expose its selected locale directly. A stable built-in label
	// distinguishes the only additional locale currently shipped by this plugin.
	const char *untitled = get_locale_string("Untitled");
	return untitled && QString::fromUtf8(untitled) == QStringLiteral("Unbenannt")
		? QStringLiteral("de-DE") : QStringLiteral("en-US");
}

void load_translations()
{
	translations.clear();
	const QString locale = obs_language().startsWith(QStringLiteral("de"), Qt::CaseInsensitive)
		? QStringLiteral("de-DE") : QStringLiteral("en-US");
	QSettings settings(QDir(module_locale_directory()).filePath(locale + QStringLiteral(".ini")),
		QSettings::IniFormat);
	for (const QString &key : settings.allKeys())
		translations.insert(key, settings.value(key).toString());
}
