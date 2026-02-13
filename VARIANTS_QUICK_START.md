# Product Variants System - Quick Start

## 🎯 Что это?

Полноценная система вариантов товаров, где **каждый вариант** может иметь:
- ✅ Свою цену (iPhone 256GB дороже чем 128GB)
- ✅ Свою картинку (черные кроссовки vs белые)
- ✅ Свой остаток (размер 42 есть, размер 43 нет)
- ✅ Свое описание (разные характеристики)

## 📊 Структура

```
Товар: iPhone 15 Pro
├── Вариант 1: 128GB Black  → 100,000₽  │ 5 шт   │ image1.jpg
├── Вариант 2: 256GB Black  → 120,000₽  │ 3 шт   │ image2.jpg
├── Вариант 3: 128GB White  → 100,000₽  │ 0 шт   │ image3.jpg  ← НЕТ В НАЛИЧИИ
└── Вариант 4: 256GB White  → 120,000₽  │ 8 шт   │ image4.jpg
```

## 🗄️ База данных

**Две таблицы:**

1. **products** - базовая информация о товаре
   - product_id, title, description, category

2. **product_variants** - варианты с ценами/остатками
   - variant_id, product_id, title, **price**, **stock_count**, **image_url**, attributes

## 🚀 Быстрый старт

### 1. Backup базы
```bash
pg_dump $DATABASE_URL > backup.sql
```

### 2. Запустить миграцию
```bash
psql $DATABASE_URL < sql/migrations/002_migrate_to_variants.sql
```

### 3. Проверить
```sql
SELECT COUNT(*) FROM products;
SELECT COUNT(*) FROM product_variants;
SELECT * FROM products_with_default_variant LIMIT 5;
```

## 💡 Примеры использования

### Создать товар с вариантами

```json
POST /api/admin/products
{
  "shop_id": 1,
  "title": "Nike Air Max",
  "category": "Shoes",
  "variants": [
    {
      "title": "Size 41 Black",
      "price": 8000,
      "stock_count": 5,
      "image_url": "nike-41-black.jpg",
      "attributes": {"size": "41", "color": "Black"}
    },
    {
      "title": "Size 42 Black",
      "price": 8000,
      "stock_count": 3,
      "image_url": "nike-42-black.jpg",
      "attributes": {"size": "42", "color": "Black"}
    }
  ]
}
```

### Получить товары (ответ с вариантами)

```json
GET /api/products/1

[
  {
    "product_id": "uuid",
    "title": "Nike Air Max",
    "category": "Shoes",
    "variants": [
      {
        "variant_id": "var-uuid-1",
        "title": "Size 41 Black",
        "price": 8000,
        "stock_count": 5,
        "image_url": "nike-41-black.jpg",
        "attributes": {"size": "41", "color": "Black"}
      },
      {
        "variant_id": "var-uuid-2",
        "title": "Size 42 Black",
        "price": 8000,
        "stock_count": 3,
        "image_url": "nike-42-black.jpg",
        "attributes": {"size": "42", "color": "Black"}
      }
    ]
  }
]
```

## ✅ Что готово

- [x] Схема базы данных
- [x] Миграционный скрипт
- [x] VIEW для удобных запросов
- [x] Индексы для производительности
- [x] Полная документация (VARIANTS_SYSTEM_GUIDE.md)
- [x] Flutter модели данных
- [x] UI компоненты

## 📚 Документация

- **VARIANTS_SYSTEM_GUIDE.md** - Полная документация (1,200+ строк)
- **sql/schema_variants.sql** - Схема БД
- **sql/migrations/002_migrate_to_variants.sql** - Миграция

## 🔧 Что дальше

**API implementation** (следующий этап):
- [ ] Обновить ProductController.cc
- [ ] Реализовать endpoints для работы с вариантами
- [ ] Обновить корзину (использовать variant_id)
- [ ] Обновить заказы (использовать variant_id)

## 💬 Примеры из реальной жизни

### Обувной магазин
```
Nike Air Max 2024
├── Размер 41, Черный - 8,000₽ - 5 шт
├── Размер 42, Черный - 8,000₽ - 3 шт
└── Размер 41, Белый - 8,500₽ - 0 шт (нет в наличии)
```

### Магазин электроники
```
iPhone 15 Pro
├── 128GB Titanium - 100,000₽ - 10 шт
├── 256GB Titanium - 120,000₽ - 5 шт
└── 512GB Titanium - 140,000₽ - 2 шт
```

### Магазин одежды
```
Supreme Hoodie
├── S, Красный - 15,000₽ - 2 шт
├── M, Красный - 15,000₽ - 5 шт
└── M, Черный - 16,000₽ - 4 шт
```

## 🎉 Готово!

Система готова к использованию. Следующий шаг - реализация API endpoints.
