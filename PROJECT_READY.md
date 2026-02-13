# 🚀 ShopKit Backend - Production Ready Guide

> **Полное руководство для разворачивания готового e-commerce бэкенда**

## 📋 Содержание

1. [Что это и для кого](#что-это-и-для-кого)
2. [Быстрый старт (5 минут)](#быстрый-старт)
3. [Архитектура проекта](#архитектура)
4. [Документация для фронтенд-разработчиков](#документация)
5. [Deployment в продакшн](#deployment)
6. [Чеклист готовности](#чеклист)

---

## 🎯 Что это и для кого

**ShopKit Backend** - готовый к продакшну бэкенд для e-commerce платформы с интеграцией Telegram Mini Apps.

### Для кого этот проект:
- ✅ **Владельцы магазинов** - управляют своим магазином через Flutter-приложение (Android/iOS)
- ✅ **Покупатели** - делают заказы через Telegram Mini App (веб-интерфейс)
- ✅ **Разработчики** - получают готовый API для быстрого старта

### Что уже готово:
- ✅ **API Backend** - C++ Drogon framework, высокая производительность
- ✅ **База данных** - PostgreSQL с оптимизированной схемой
- ✅ **13 API endpoints** - полный функционал магазина
- ✅ **Документация** - для TypeScript TMA и Flutter
- ✅ **Production deployment** - развернуто на Render.com
- ✅ **Варианты товаров** - поддержка характеристик (размер, цвет, модель)

---

## ⚡ Быстрый старт

### Для разработчика фронтенда (начни здесь!)

**1. Выбери свою платформу:**

| Платформа | Кто использует | Документация |
|-----------|----------------|--------------|
| **TypeScript + React** | Telegram Mini App (покупатели) | [FRONTEND_COMPLETE_GUIDE.md](./FRONTEND_COMPLETE_GUIDE.md) |
| **Flutter** | Admin-приложение (владельцы магазинов) | [FLUTTER_FULL_GUIDE.md](./FLUTTER_FULL_GUIDE.md) |
| **API Reference** | Все разработчики | [API_ENDPOINTS.md](./API_ENDPOINTS.md) |

**2. Production API URL:**
```
https://shopkit-backend.onrender.com/api
```

**3. Тестовый запрос:**
```bash
# Проверка работоспособности
curl https://shopkit-backend.onrender.com/api/shops/token/test_shop

# Получение списка товаров (если магазин с shop_id=1 существует)
curl https://shopkit-backend.onrender.com/api/products/1
```

### Для владельца проекта (деплой бэкенда)

**1. Склонируй репозиторий:**
```bash
git clone https://github.com/iliaatmazhitov/shopkit_backend.git
cd shopkit_backend
```

**2. Выбери способ деплоя:**

**Вариант A: Render.com (уже настроен)**
- Форкни репозиторий
- Подключи к Render.com
- Добавь PostgreSQL
- Deploy автоматически

**Вариант B: Docker**
```bash
# Создай .env файл
cp .env.example .env
# Отредактируй DATABASE_URL

# Запусти
docker-compose up -d
```

**Вариант C: Railway**
```bash
railway login
railway init
railway add postgresql
railway up
```

---

## 🏗️ Архитектура

### Компоненты системы

```
┌─────────────────────────────────────────────────────────┐
│                    TELEGRAM ECOSYSTEM                    │
├─────────────────────────────────────────────────────────┤
│                                                           │
│  ┌──────────────────┐         ┌──────────────────┐      │
│  │  Telegram Bot    │         │   Flutter App    │      │
│  │  (Mini App)      │         │  (Admin Panel)   │      │
│  │                  │         │                  │      │
│  │  Покупатели      │         │  Владельцы       │      │
│  │  заказывают      │         │  управляют       │      │
│  └────────┬─────────┘         └────────┬─────────┘      │
│           │                            │                 │
│           │  TypeScript/React          │  Flutter/Dart   │
│           │                            │                 │
└───────────┼────────────────────────────┼─────────────────┘
            │                            │
            ▼                            ▼
   ┌────────────────────────────────────────────┐
   │         ShopKit Backend API                │
   │         (C++ Drogon Framework)             │
   │                                            │
   │  ┌──────────────────────────────────────┐ │
   │  │  13 REST API Endpoints               │ │
   │  │  - Shop Management                   │ │
   │  │  - Product CRUD + Attributes         │ │
   │  │  - Cart (server-side sync)           │ │
   │  │  - Orders + History                  │ │
   │  └──────────────────────────────────────┘ │
   │                                            │
   │  ┌──────────────────────────────────────┐ │
   │  │  Features:                           │ │
   │  │  - CORS configured                   │ │
   │  │  - JSON API                          │ │
   │  │  - Error handling                    │ │
   │  │  - Authentication (Telegram IDs)     │ │
   │  └──────────────────────────────────────┘ │
   └────────────────┬───────────────────────────┘
                    │
                    ▼
           ┌────────────────┐
           │   PostgreSQL   │
           │    Database    │
           │                │
           │  Tables:       │
           │  - shops       │
           │  - products    │
           │  - orders      │
           │  - carts       │
           └────────────────┘
```

### Таблицы базы данных

**Основные таблицы:**

1. **shops** - Магазины
   - shop_token (уникальный токен для доступа)
   - owner_tg_id (Telegram ID владельца)
   - title, description, currency

2. **products** - Товары
   - product_id (UUID)
   - shop_id
   - title, price, stock_count
   - image_url, description
   - **attributes** (JSONB) - гибкие характеристики
   - category, tags

3. **orders** - Заказы
   - order_id (UUID)
   - shop_id, telegram_user_id
   - customer_name, customer_phone
   - items (JSONB массив товаров)
   - total_price, status
   - payment_method, notes

4. **carts** - Корзины (server-side)
   - telegram_user_id, shop_id
   - items (JSONB)
   - updated_at

### API Endpoints (13 total)

**Shop Management (3):**
- `GET /api/shops/token/{token}` - Получить магазин
- `POST /api/shops` - Создать магазин
- `PUT /api/shops/{token}` - Обновить магазин

**Product Management (5):**
- `GET /api/products/{shop_id}` - Все товары
- `GET /api/products/detail/{product_id}` - Детали товара
- `POST /api/admin/products` - Создать товар (requires X-Owner-TG-ID)
- `PUT /api/admin/products/{id}` - Обновить товар (requires X-Owner-TG-ID)
- `DELETE /api/admin/products/{id}` - Удалить товар (requires X-Owner-TG-ID)

**Cart Management (3):**
- `POST /api/cart` - Сохранить корзину
- `GET /api/cart/{user_id}?shop_id={id}` - Получить корзину
- `DELETE /api/cart/{user_id}?shop_id={id}` - Очистить корзину

**Order Management (2):**
- `POST /api/orders` - Создать заказ
- `GET /api/user/orders/{user_id}` - Все заказы пользователя

---

## 📚 Документация

### Для TypeScript / React разработчиков (TMA)

**Telegram Mini App для покупателей**

📖 **[FRONTEND_COMPLETE_GUIDE.md](./FRONTEND_COMPLETE_GUIDE.md)** - Полное руководство (2,474 строки)

**Что внутри:**
- ✅ Полная настройка Telegram Mini App
- ✅ TypeScript типы для всех моделей
- ✅ API клиент с retry logic и error handling
- ✅ React hooks (useCart, useShop, useProducts, useOrders)
- ✅ Готовые компоненты страниц (ShopPage, CartPage, CheckoutPage, OrdersPage)
- ✅ Интеграция с Telegram WebApp (MainButton, BackButton, темы)
- ✅ Примеры кода (ready to copy-paste)

**Быстрый старт:**
```typescript
// 1. Установи @twa-dev/sdk
npm install @twa-dev/sdk

// 2. Скопируй типы из документации
interface Product { ... }

// 3. Используй готовый API клиент
const api = new ShopKitAPI('https://shopkit-backend.onrender.com/api');
const products = await api.getProducts(shopId);

// 4. Используй готовые хуки
const { cart, addToCart, removeFromCart } = useCart(userId, shopId);
```

### Для Flutter разработчиков (Admin App)

**Flutter-приложение для владельцев магазинов (Android + iOS)**

📖 **[FLUTTER_FULL_GUIDE.md](./FLUTTER_FULL_GUIDE.md)** - Объединенное руководство

**Что внутри:**
- ✅ Настройка Flutter проекта
- ✅ Dart модели (Shop, Product, Order)
- ✅ ApiService с полной реализацией
- ✅ State Management (Provider)
- ✅ Полные UI экраны:
  - AuthScreen (вход по Telegram ID)
  - ShopSetupScreen (создание/настройка магазина)
  - ProductsListScreen (список товаров)
  - ProductFormScreen (добавление/редактирование товара)
  - OrdersListScreen (заказы клиентов)
- ✅ Deployment на Android Play Store и iOS App Store
- ✅ Примеры кода (ready to copy-paste)

**Быстрый старт:**
```dart
// 1. Создай Flutter проект
flutter create shop_admin

// 2. Добавь зависимости в pubspec.yaml
dependencies:
  http: ^1.1.0
  provider: ^6.0.5
  shared_preferences: ^2.2.0

// 3. Скопируй модели из документации
class Product { ... }

// 4. Используй готовый ApiService
final api = ApiService();
final products = await api.getProducts(shopId);

// 5. Используй Provider для state management
ProductsProvider, ShopProvider, OrdersProvider
```

### Справочник API

📖 **[API_ENDPOINTS.md](./API_ENDPOINTS.md)** - Быстрый справочник (818 строк)

**Формат каждого endpoint:**
```
### Название

GET/POST/PUT/DELETE /api/endpoint/{params}

Request Body: {JSON с типами}
Response 200: {JSON пример}
Response 400/404: {JSON ошибки}

Примеры: cURL + JavaScript
```

---

## 🚀 Deployment

### Production Deployment (Render.com)

**Текущий статус:** ✅ Уже развернуто на https://shopkit-backend.onrender.com/api

**Как обновить/переразвернуть:**

1. **Форкни репозиторий** на свой GitHub

2. **Создай новый Web Service на Render.com:**
   - Build Command: `cd shopkit && mkdir -p build && cd build && cmake .. && make -j4`
   - Start Command: `cd shopkit/build && ./shopkit`
   - Environment: Docker (или Native если есть C++ support)

3. **Добавь PostgreSQL database** на Render.com:
   - Создай PostgreSQL instance
   - Скопируй Internal Database URL

4. **Настрой Environment Variables:**
   ```
   DATABASE_URL=postgresql://user:pass@host:5432/dbname
   PORT=8080
   ```

5. **Примени схему базы данных:**
   ```bash
   # Через Render Shell или локально
   psql $DATABASE_URL < sql/schema.sql
   ```

6. **Deploy!** - Render автоматически соберет и запустит проект

### Docker Deployment

**Создай `.env` файл:**
```env
DATABASE_URL=postgresql://user:pass@postgres:5432/shopkit
PORT=8080
```

**Запусти через Docker Compose:**
```bash
docker-compose up -d
```

**Проверь:**
```bash
curl http://localhost:8080/api/shops/token/test
```

### Railway Deployment

```bash
# 1. Установи Railway CLI
npm install -g @railway/cli

# 2. Логин и инициализация
railway login
railway init

# 3. Добавь PostgreSQL
railway add postgresql

# 4. Deploy
railway up

# 5. Примени схему
railway run psql $DATABASE_URL -f sql/schema.sql

# 6. Получи URL
railway domain
```

---

## ✅ Чеклист готовности

### Для продакшн-деплоя

**Backend:**
- [x] База данных настроена (PostgreSQL)
- [x] Схема применена (`sql/schema.sql`)
- [x] API работает (13 endpoints)
- [x] CORS настроен для всех origins
- [x] Error handling реализован
- [x] Логирование настроено

**Frontend TMA (TypeScript/React):**
- [ ] Установлен @twa-dev/sdk
- [ ] Скопированы TypeScript типы
- [ ] API клиент интегрирован
- [ ] Компоненты реализованы
- [ ] Telegram Bot создан
- [ ] Start parameter настроен (shop_token)

**Frontend Admin (Flutter):**
- [ ] Flutter проект создан
- [ ] Зависимости установлены
- [ ] Dart модели скопированы
- [ ] ApiService интегрирован
- [ ] Screens реализованы
- [ ] Android build настроен
- [ ] iOS build настроен (если нужно)

**Deployment:**
- [x] Production URL работает
- [ ] Environment variables настроены
- [ ] Database migrations применены
- [ ] SSL/HTTPS настроен (через Render/Railway)
- [ ] Мониторинг настроен (опционально)

### Testing Checklist

```bash
# 1. Проверка API
curl https://shopkit-backend.onrender.com/api/shops/token/test

# 2. Создание магазина
curl -X POST https://shopkit-backend.onrender.com/api/shops \
  -H "Content-Type: application/json" \
  -d '{"owner_tg_id": 123456789, "title": "Test Shop", "currency": "RUB"}'

# 3. Добавление товара
curl -X POST https://shopkit-backend.onrender.com/api/admin/products \
  -H "Content-Type: application/json" \
  -H "X-Owner-TG-ID: 123456789" \
  -d '{"shop_id": 1, "title": "iPhone", "price": 100000, "stock_count": 5}'

# 4. Получение товаров
curl https://shopkit-backend.onrender.com/api/products/1

# 5. Создание заказа
curl -X POST https://shopkit-backend.onrender.com/api/orders \
  -H "Content-Type: application/json" \
  -d '{
    "shop_id": 1,
    "telegram_user_id": 987654321,
    "customer_name": "Test User",
    "items": [{"product_id": "uuid", "title": "iPhone", "quantity": 1, "price": 100000}],
    "total_price": 100000
  }'

# 6. Получение заказов
curl https://shopkit-backend.onrender.com/api/user/orders/987654321
```

---

## 🎯 Roadmap и улучшения

**Что уже есть:**
- ✅ Полный CRUD для магазинов, товаров, заказов
- ✅ Server-side корзины
- ✅ Поддержка атрибутов товаров (размер, цвет, модель)
- ✅ История заказов пользователя across all shops
- ✅ Admin authentication (X-Owner-TG-ID header)

**Что можно добавить (опционально):**
- [ ] Загрузка изображений (endpoint для upload)
- [ ] Webhook уведомления (при новом заказе)
- [ ] Платежная интеграция (Telegram Stars, Stripe)
- [ ] Статистика магазина (dashboard)
- [ ] Поиск и фильтры товаров
- [ ] Отзывы и рейтинги
- [ ] Push-уведомления
- [ ] Analytics

---

## 📞 Support

**Документация:**
- [API Endpoints](./API_ENDPOINTS.md) - Справочник API
- [TypeScript Guide](./FRONTEND_COMPLETE_GUIDE.md) - Telegram Mini App
- [Flutter Guide](./FLUTTER_FULL_GUIDE.md) - Admin приложение

**GitHub:**
- Issues: https://github.com/iliaatmazhitov/shopkit_backend/issues

**Production API:**
- Base URL: https://shopkit-backend.onrender.com/api
- Status: ✅ Live and operational

---

## 📄 License

MIT License - используй свободно в своих проектах.

---

**Готово к продакшну! 🚀**

Все компоненты протестированы, задокументированы и готовы к деплою.
