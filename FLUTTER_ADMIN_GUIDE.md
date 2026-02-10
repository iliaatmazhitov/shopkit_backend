# 📱 Flutter Admin App - Руководство для владельцев магазинов

> **Production API:** `https://shopkit-backend.onrender.com/api`  
> **Status:** ✅ Production-ready

Полное руководство по разработке **admin приложения** на Flutter для владельцев магазинов. Приложение позволяет управлять магазином, добавлять товары, просматривать заказы.

## 🎯 Что это за приложение?

Это **админ-панель для владельцев магазинов**, которая позволяет:
- ✅ Создать свой магазин
- ✅ Добавлять, редактировать и удалять товары
- ✅ Загружать фотографии товаров
- ✅ Управлять остатками на складе
- ✅ Просматривать заказы клиентов
- ✅ Изменять статусы заказов
- ✅ Редактировать информацию о магазине

**Это НЕ приложение для покупателей!** Это приложение для тех, кто управляет магазином.

---

## Содержание

1. [Quick Start](#quick-start)
2. [Аутентификация](#аутентификация)
3. [Структура проекта](#структура-проекта)
4. [Модели данных](#модели-данных)
5. [Admin API Service](#admin-api-service)
6. [Управление магазином](#управление-магазином)
7. [Управление товарами](#управление-товарами)
8. [Загрузка изображений](#загрузка-изображений)
9. [UI Компоненты](#ui-компоненты)
10. [Деплой](#деплой)

---

## Quick Start

### Требования

- Flutter SDK 3.16.0+
- Dart 3.2.0+
- Android Studio / Xcode
- Telegram Bot (для получения owner_tg_id)

### Создание проекта

```bash
flutter create shopkit_admin
cd shopkit_admin

# Добавить зависимости
flutter pub add http provider shared_preferences image_picker
flutter pub add --dev flutter_lints

flutter run
```

### pubspec.yaml

```yaml
name: shopkit_admin
description: Admin app for shop owners
version: 1.0.0+1

environment:
  sdk: ">=3.2.0 <4.0.0"

dependencies:
  flutter:
    sdk: flutter
  
  # HTTP и State Management
  http: ^1.1.0
  provider: ^6.1.1
  
  # Работа с изображениями
  image_picker: ^1.0.5
  
  # Локальное хранилище
  shared_preferences: ^2.2.2
  
  # UI helpers
  intl: ^0.18.1

dev_dependencies:
  flutter_test:
    sdk: flutter
  flutter_lints: ^3.0.0
```

---

## Аутентификация

### Как получить owner_tg_id

Владелец магазина должен получить свой Telegram User ID:

1. Открыть Telegram
2. Найти бота `@userinfobot`
3. Отправить ему любое сообщение
4. Бот пришлёт ваш Telegram ID (например: `123456789`)

Этот ID используется для:
- Создания магазина
- Аутентификации admin операций (header `X-Owner-TG-ID`)

---

## Admin Endpoints

### Требуют аутентификации (X-Owner-TG-ID):
- ✅ POST /api/admin/products - создать товар
- ✅ PUT /api/admin/products/{id} - обновить товар
- ✅ DELETE /api/admin/products/{id} - удалить товар
- ✅ PUT /api/shops/{token} - обновить магазин

### Публичные (но используются admin-ом):
- ✅ POST /api/shops - создать магазин
- ✅ GET /api/shops/token/{token} - получить магазин
- ✅ GET /api/products/{shop_id} - получить товары

---

## Основные отличия от клиентского приложения

### ❌ Клиентское приложение (для покупателей):
- Просмотр товаров
- Добавление в корзину
- Оформление заказов
- БЕЗ X-Owner-TG-ID

### ✅ Admin приложение (для владельцев):
- Создание магазина
- Добавление/редактировать/удаление товаров
- Просмотр заказов клиентов
- Управление остатками
- **Требует X-Owner-TG-ID для admin операций**

---

Для полной документации по models, API service, UI components и deployment смотрите [FLUTTER_GUIDE.md](./FLUTTER_GUIDE.md) и адаптируйте под admin нужды.

**Ключевое отличие:** В Admin приложении используется header `X-Owner-TG-ID` для всех операций с товарами.

---

**Готово!** Теперь у вас есть руководство по созданию admin приложения для владельцев магазинов на Flutter! 🚀
