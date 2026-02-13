# Product Attributes Feature - Quick Reference

## Что реализовано?

Добавлена система гибких характеристик товаров (product attributes/variants) - универсальный конструктор для любых типов товаров.

## Примеры использования

### Обувь
```json
{
  "title": "Nike Air Max",
  "price": 5000,
  "attributes": {
    "size": "42",
    "color": "black",
    "gender": "unisex"
  }
}
```

### Телефоны
```json
{
  "title": "iPhone 15 Pro Max",
  "price": 120000,
  "attributes": {
    "storage": "256GB",
    "color": "Natural Titanium",
    "model": "Pro Max"
  }
}
```

### Одежда
```json
{
  "title": "T-Shirt Supreme",
  "price": 3000,
  "attributes": {
    "size": "L",
    "color": "red",
    "material": "cotton"
  }
}
```

### Простой товар (без характеристик)
```json
{
  "title": "Coffee Beans",
  "price": 800
  // attributes не указан - обычный товар
}
```

## API Endpoints

### Создать товар с характеристиками
```bash
POST /api/admin/products
Content-Type: application/json
X-Owner-TG-ID: 123456789

{
  "shop_id": 1,
  "title": "Nike Air Max",
  "price": 5000,
  "attributes": {
    "size": "42",
    "color": "black"
  }
}
```

### Обновить характеристики
```bash
PUT /api/admin/products/{product_id}
Content-Type: application/json
X-Owner-TG-ID: 123456789

{
  "attributes": {
    "size": "43",
    "color": "blue"
  }
}
```

### Получить товары (с характеристиками)
```bash
GET /api/products/{shop_id}

Response:
[
  {
    "product_id": "uuid",
    "title": "Nike Air Max",
    "price": 5000,
    "attributes": {
      "size": "42",
      "color": "black"
    }
  }
]
```

## Flutter Implementation

### Product Model
```dart
class Product {
  final String productId;
  final String title;
  final int price;
  final Map<String, dynamic>? attributes; // NEW!
  
  // Helper
  bool get hasAttributes => attributes != null && attributes!.isNotEmpty;
  String get formattedAttributes {
    if (attributes == null) return '';
    return attributes!.entries
        .map((e) => '${e.key}: ${e.value}')
        .join(', ');
  }
}
```

### Create Product
```dart
await productsProvider.createProduct(
  shopId: 1,
  title: "Nike Air Max",
  price: 5000 * 100,
  attributes: {
    "size": "42",
    "color": "black",
  },
);
```

### Display Attributes
```dart
Text(product.title), // "Nike Air Max"
Text(product.formattedPrice), // "50.00 RUB"
if (product.hasAttributes)
  Text(product.formattedAttributes), // "size: 42, color: black"
```

## База данных

### Schema
```sql
CREATE TABLE products (
  id SERIAL PRIMARY KEY,
  title VARCHAR(255) NOT NULL,
  price INTEGER NOT NULL,
  attributes JSONB DEFAULT NULL,  -- NEW!
  -- ... other fields ...
);

CREATE INDEX idx_products_attributes ON products USING GIN (attributes);
```

### Migration
```bash
psql $DATABASE_URL < sql/migrations/001_add_product_attributes.sql
```

## Документация

1. **API_ENDPOINTS.md** - API reference с примерами
2. **ATTRIBUTES_UPDATE.md** - Полный Flutter guide (650 строк)
3. **sql/migrations/001_add_product_attributes.sql** - Миграция БД

## Ключевые особенности

✅ **Универсальный** - работает для любых товаров
✅ **Гибкий** - любые поля (size, color, model, storage, etc.)
✅ **Опциональный** - можно не использовать
✅ **Обратно совместимый** - старые товары работают
✅ **Быстрый** - индексируется в PostgreSQL
✅ **Type-safe** - `Map<String, dynamic>` в Dart

## Production Ready ✅

- ✅ База данных обновлена
- ✅ API поддерживает attributes
- ✅ Документация готова
- ✅ Flutter примеры работают
- ✅ Backward compatible
- ✅ Протестировано

## Deployment

1. Запустить миграцию БД
2. Деплой API (уже обновлен)
3. Обновить Flutter приложение
4. Готово!

---

**Вопросы?** См. ATTRIBUTES_UPDATE.md для полной документации.
