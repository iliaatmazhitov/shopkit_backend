# API Endpoints Reference

> **Production API Base:** `https://shopkit-backend.onrender.com/api`

Простой справочник всех API endpoints с форматом запросов и ответов.

---

## 🏪 Shop Endpoints

### 1. Получить магазин по токену

```
GET /api/shops/token/{shop_token}
```

**Параметры URL:**
- `shop_token` (string) - токен магазина

**Response 200:**
```json
{
  "id": 1,
  "title": "My Coffee Shop",
  "description": "Best coffee in town",
  "currency": "RUB",
  "shop_token": "my_coffee_shop_1234",
  "created_at": "2024-01-01T10:00:00",
  "products_count": 15
}
```

**Response 404:**
```json
{
  "error": "Shop not found",
  "code": "SHOP_NOT_FOUND"
}
```

---

### 2. Создать магазин

```
POST /api/shops
```

**Request Body:**
```json
{
  "owner_tg_id": 123456789,           // обязательно - Telegram User ID владельца
  "title": "My Shop",                 // обязательно - название магазина
  "description": "Shop description",  // опционально
  "currency": "RUB",                  // опционально, по умолчанию "RUB"
  "shop_token": "custom_token"        // опционально, будет сгенерирован автоматически
}
```

**Response 201:**
```json
{
  "success": true,
  "shop": {
    "id": 1,
    "shop_token": "my_shop_6789",
    "title": "My Shop",
    "created_at": "2024-01-01T10:00:00"
  },
  "message": "Shop created successfully"
}
```

**Response 400:**
```json
{
  "error": "Missing required fields: owner_tg_id and title"
}
```

---

### 3. Обновить магазин

```
PUT /api/shops/{shop_token}
```

**Headers:**
```
X-Owner-TG-ID: 123456789  // обязательно - Telegram User ID владельца
```

**Параметры URL:**
- `shop_token` (string) - токен магазина

**Request Body:**
```json
{
  "title": "New Shop Name",          // опционально
  "description": "New description",   // опционально
  "currency": "USD"                   // опционально
}
```

**Response 200:**
```json
{
  "success": true,
  "shop_token": "my_shop_6789",
  "title": "New Shop Name",
  "currency": "USD",
  "updated_at": "2024-01-01T11:00:00"
}
```

**Response 401:**
```json
{
  "error": "Authentication required",
  "code": "AUTH_REQUIRED"
}
```

**Response 403:**
```json
{
  "error": "Access denied",
  "code": "FORBIDDEN"
}
```

---

## 📦 Product Endpoints

### 4. Получить все товары магазина

```
GET /api/products/{shop_id}
```

**Параметры URL:**
- `shop_id` (integer) - ID магазина

**Response 200:**
```json
[
  {
    "product_id": "550e8400-e29b-41d4-a716-446655440000",
    "title": "Espresso",
    "price": 250,
    "currency": "RUB",
    "image_url": "https://example.com/image.jpg",
    "description": "Rich and bold espresso",
    "stock_count": 100,
    "category": "Coffee"
  },
  {
    "product_id": "660e8400-e29b-41d4-a716-446655440001",
    "title": "Cappuccino",
    "price": 350,
    "currency": "RUB",
    "image_url": "",
    "description": "",
    "stock_count": 50,
    "category": "Coffee"
  }
]
```

---

### 5. Получить детали товара

```
GET /api/products/detail/{product_id}
```

**Параметры URL:**
- `product_id` (string, UUID) - ID товара

**Response 200:**
```json
{
  "product_id": "550e8400-e29b-41d4-a716-446655440000",
  "title": "Espresso",
  "price": 250,
  "currency": "RUB",
  "image_url": "https://example.com/image.jpg",
  "description": "Rich and bold espresso",
  "stock_count": 100,
  "category": "Coffee",
  "shop": {
    "id": 1,
    "title": "My Coffee Shop",
    "shop_token": "my_coffee_shop_1234"
  }
}
```

**Response 404:**
```json
{
  "error": "Product not found",
  "code": "PRODUCT_NOT_FOUND"
}
```

---

### 6. Создать товар (admin)

```
POST /api/admin/products
```

**Headers:**
```
X-Owner-TG-ID: 123456789  // обязательно - Telegram User ID владельца магазина
```

**Request Body:**
```json
{
  "shop_id": 1,                       // обязательно
  "title": "Espresso",                // обязательно
  "price": 250,                       // обязательно (в копейках/центах)
  "currency": "RUB",                  // опционально, по умолчанию "RUB"
  "image_url": "https://...",         // опционально
  "description": "Description",       // опционально
  "stock_count": 100,                 // опционально, по умолчанию 0
  "category": "Coffee"                // опционально
}
```

**Response 201:**
```json
{
  "success": true,
  "product": {
    "product_id": "550e8400-e29b-41d4-a716-446655440000",
    "shop_id": 1,
    "title": "Espresso",
    "price": 250,
    "currency": "RUB",
    "created_at": "2024-01-01T10:00:00"
  }
}
```

**Response 400:**
```json
{
  "error": "Missing required fields: shop_id, title, price"
}
```

---

### 7. Обновить товар (admin)

```
PUT /api/admin/products/{product_id}
```

**Headers:**
```
X-Owner-TG-ID: 123456789  // обязательно
```

**Параметры URL:**
- `product_id` (string, UUID) - ID товара

**Request Body:**
```json
{
  "title": "New Title",           // опционально
  "price": 300,                   // опционально
  "description": "New desc",      // опционально
  "image_url": "https://...",     // опционально
  "stock_count": 150,             // опционально
  "category": "New Category"      // опционально
}
```

**Response 200:**
```json
{
  "success": true,
  "product_id": "550e8400-e29b-41d4-a716-446655440000",
  "title": "New Title",
  "price": 300,
  "updated_at": "2024-01-01T11:00:00"
}
```

**Response 404:**
```json
{
  "error": "Product not found"
}
```

---

### 8. Удалить товар (admin)

```
DELETE /api/admin/products/{product_id}
```

**Headers:**
```
X-Owner-TG-ID: 123456789  // обязательно
```

**Параметры URL:**
- `product_id` (string, UUID) - ID товара

**Response 200:**
```json
{
  "success": true,
  "product_id": "550e8400-e29b-41d4-a716-446655440000",
  "message": "Product deleted successfully"
}
```

**Response 404:**
```json
{
  "error": "Product not found"
}
```

---

## 🛒 Cart Endpoints

### 9. Сохранить корзину

```
POST /api/cart
```

**Request Body:**
```json
{
  "telegram_user_id": 123456789,      // обязательно - Telegram User ID
  "shop_id": 1,                       // обязательно - ID магазина
  "items": [                          // обязательно - массив товаров
    {
      "product_id": "550e8400-e29b-41d4-a716-446655440000",
      "title": "Espresso",
      "quantity": 2,
      "price": 250
    },
    {
      "product_id": "660e8400-e29b-41d4-a716-446655440001",
      "title": "Cappuccino",
      "quantity": 1,
      "price": 350
    }
  ]
}
```

**Response 200:**
```json
{
  "success": true,
  "updated_at": "2024-01-01T10:00:00"
}
```

**Response 400:**
```json
{
  "error": "Missing required fields: telegram_user_id, shop_id, items"
}
```

---

### 10. Получить корзину

```
GET /api/cart/{telegram_user_id}?shop_id={shop_id}
```

**Параметры URL:**
- `telegram_user_id` (integer) - Telegram User ID

**Query Parameters:**
- `shop_id` (integer) - ID магазина (обязательно)

**Response 200:**
```json
{
  "telegram_user_id": 123456789,
  "shop_id": 1,
  "items": [
    {
      "product_id": "550e8400-e29b-41d4-a716-446655440000",
      "title": "Espresso",
      "quantity": 2,
      "price": 250
    }
  ],
  "updated_at": "2024-01-01T10:00:00"
}
```

**Response 200 (пустая корзина):**
```json
{
  "telegram_user_id": 123456789,
  "shop_id": 1,
  "items": []
}
```

**Response 400:**
```json
{
  "error": "Missing required parameter: shop_id"
}
```

---

### 11. Очистить корзину

```
DELETE /api/cart/{telegram_user_id}?shop_id={shop_id}
```

**Параметры URL:**
- `telegram_user_id` (integer) - Telegram User ID

**Query Parameters:**
- `shop_id` (integer) - ID магазина (обязательно)

**Response 200:**
```json
{
  "success": true
}
```

**Response 400:**
```json
{
  "error": "Missing required parameter: shop_id"
}
```

---

## 📋 Order Endpoints

### 12. Создать заказ

```
POST /api/orders
```

**Request Body:**
```json
{
  "shop_id": 1,                       // обязательно - ID магазина
  "telegram_user_id": 123456789,      // опционально - Telegram User ID
  "customer_name": "John Doe",        // опционально - имя покупателя
  "customer_phone": "+1234567890",    // опционально - телефон
  "items": [                          // обязательно - массив товаров
    {
      "product_id": "550e8400-e29b-41d4-a716-446655440000",
      "title": "Espresso",
      "quantity": 2,
      "price": 250
    }
  ],
  "total_price": 500,                 // обязательно - общая сумма (в копейках)
  "currency": "RUB",                  // обязательно - валюта
  "status": "pending",                // опционально, по умолчанию "pending"
  "payment_method": "cash",           // опционально - метод оплаты
  "notes": "Please call before"       // опционально - примечания
}
```

**Response 201:**
```json
{
  "success": true,
  "order": {
    "order_id": "770e8400-e29b-41d4-a716-446655440002",
    "shop_id": 1,
    "total_price": 500,
    "currency": "RUB",
    "status": "pending",
    "items": [
      {
        "product_id": "550e8400-e29b-41d4-a716-446655440000",
        "title": "Espresso",
        "quantity": 2,
        "price": 250
      }
    ],
    "created_at": "2024-01-01T10:00:00"
  }
}
```

**Response 400:**
```json
{
  "error": "Missing required fields: shop_id, items, total_price, currency"
}
```

---

### 13. Получить все заказы пользователя

```
GET /api/user/orders/{telegram_user_id}
```

**Параметры URL:**
- `telegram_user_id` (integer) - Telegram User ID

**Response 200:**
```json
[
  {
    "order_id": "770e8400-e29b-41d4-a716-446655440002",
    "shop_id": 1,
    "shop_title": "My Coffee Shop",
    "shop_token": "my_coffee_shop_1234",
    "total_price": 500,
    "currency": "RUB",
    "status": "pending",
    "items": [
      {
        "product_id": "550e8400-e29b-41d4-a716-446655440000",
        "title": "Espresso",
        "quantity": 2,
        "price": 250
      }
    ],
    "created_at": "2024-01-01T10:00:00"
  },
  {
    "order_id": "880e8400-e29b-41d4-a716-446655440003",
    "shop_id": 2,
    "shop_title": "Another Shop",
    "shop_token": "another_shop_5678",
    "total_price": 1000,
    "currency": "RUB",
    "status": "completed",
    "items": [...],
    "created_at": "2024-01-02T12:00:00"
  }
]
```

**Примечание:** Возвращает заказы из ВСЕХ магазинов пользователя, отсортированные по дате создания (новые первые).

---

## 📝 Notes

### Типы данных

- **integer** - целое число
- **string** - строка текста
- **UUID** - уникальный идентификатор формата `550e8400-e29b-41d4-a716-446655440000`
- **array** - массив объектов

### Коды ошибок

- **200 OK** - успешный запрос
- **201 Created** - ресурс успешно создан
- **400 Bad Request** - неверный формат запроса
- **401 Unauthorized** - требуется аутентификация
- **403 Forbidden** - доступ запрещён
- **404 Not Found** - ресурс не найден
- **500 Internal Server Error** - ошибка сервера

### Аутентификация

Admin endpoints (создание/обновление/удаление товаров, обновление магазина) требуют header:
```
X-Owner-TG-ID: {telegram_user_id владельца магазина}
```

### CORS

API поддерживает CORS для всех origins - можно делать запросы из любого домена.

### Цены

Все цены указываются в **минимальных единицах валюты** (копейки для рублей, центы для долларов):
- 250 = 2.50 RUB
- 1000 = 10.00 RUB
- 99 = 0.99 USD

---

## 🔧 Примеры использования

### cURL Examples

```bash
# Получить магазин
curl https://shopkit-backend.onrender.com/api/shops/token/my_shop_123

# Создать товар
curl -X POST https://shopkit-backend.onrender.com/api/admin/products \
  -H "Content-Type: application/json" \
  -H "X-Owner-TG-ID: 123456789" \
  -d '{
    "shop_id": 1,
    "title": "Espresso",
    "price": 250,
    "currency": "RUB",
    "stock_count": 100
  }'

# Сохранить корзину
curl -X POST https://shopkit-backend.onrender.com/api/cart \
  -H "Content-Type: application/json" \
  -d '{
    "telegram_user_id": 123456789,
    "shop_id": 1,
    "items": [
      {"product_id":"550e8400-e29b-41d4-a716-446655440000","title":"Espresso","quantity":2,"price":250}
    ]
  }'

# Получить заказы пользователя
curl https://shopkit-backend.onrender.com/api/user/orders/123456789
```

### JavaScript/TypeScript Example

```typescript
const API_BASE = 'https://shopkit-backend.onrender.com/api';

// Получить магазин
const shop = await fetch(`${API_BASE}/shops/token/my_shop_123`).then(r => r.json());

// Получить товары
const products = await fetch(`${API_BASE}/products/${shop.id}`).then(r => r.json());

// Сохранить корзину
await fetch(`${API_BASE}/cart`, {
  method: 'POST',
  headers: { 'Content-Type': 'application/json' },
  body: JSON.stringify({
    telegram_user_id: 123456789,
    shop_id: shop.id,
    items: [
      { product_id: products[0].product_id, title: products[0].title, quantity: 2, price: products[0].price }
    ]
  })
});

// Создать заказ
const order = await fetch(`${API_BASE}/orders`, {
  method: 'POST',
  headers: { 'Content-Type': 'application/json' },
  body: JSON.stringify({
    shop_id: shop.id,
    telegram_user_id: 123456789,
    customer_name: 'John Doe',
    items: [...],
    total_price: 500,
    currency: 'RUB'
  })
}).then(r => r.json());
```

---

## 📚 Дополнительная документация

Для более подробной информации и примеров интеграции смотрите:
- [FRONTEND_COMPLETE_GUIDE.md](./FRONTEND_COMPLETE_GUIDE.md) - полное руководство с React hooks и компонентами
- [examples/](./examples/) - готовые примеры кода
