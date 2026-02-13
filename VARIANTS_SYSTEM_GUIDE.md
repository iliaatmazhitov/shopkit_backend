# Product Variants System - Complete Guide

## 🎯 Overview

Полная система вариантов товаров, где каждый вариант может иметь:
- ✅ Свою цену
- ✅ Свою картинку  
- ✅ Свой остаток на складе
- ✅ Свое описание
- ✅ Свои характеристики (attributes)

## 📊 Архитектура

### Старая система (Simple Attributes)
```
products table:
- product_id
- title: "iPhone 15 Pro"
- price: 100000 (одна цена для всех)
- stock_count: 5 (один остаток)
- image_url: "iphone.jpg" (одна картинка)
- attributes: {"storage": "128GB"} (просто метаданные)
```

**Проблема:** Нельзя иметь разные цены/картинки/остатки для разных размеров/цветов/моделей.

### Новая система (Product Variants)
```
products table (базовая информация):
- product_id: "uuid-1"
- title: "iPhone 15 Pro"
- description: "Latest iPhone"
- category: "Electronics"

product_variants table (каждый вариант):
1. variant_id: "var-1"
   title: "iPhone 15 Pro 128GB Black"
   price: 100000
   stock_count: 5
   image_url: "iphone-128-black.jpg"
   attributes: {"storage": "128GB", "color": "Black"}

2. variant_id: "var-2"
   title: "iPhone 15 Pro 256GB Black"
   price: 120000
   stock_count: 3
   image_url: "iphone-256-black.jpg"
   attributes: {"storage": "256GB", "color": "Black"}

3. variant_id: "var-3"
   title: "iPhone 15 Pro 128GB White"
   price: 100000
   stock_count: 0 ← НЕТ В НАЛИЧИИ
   image_url: "iphone-128-white.jpg"
   attributes: {"storage": "128GB", "color": "White"}
```

**Преимущества:** Полная гибкость, каждый вариант независим.

## 🗄️ Database Schema

### products table (родительские товары)
```sql
CREATE TABLE products (
    id SERIAL PRIMARY KEY,
    product_id UUID DEFAULT gen_random_uuid() UNIQUE,
    shop_id INTEGER NOT NULL REFERENCES shops(id),
    title VARCHAR(255) NOT NULL,           -- Базовое название
    description TEXT,                       -- Базовое описание
    category VARCHAR(100),
    tags TEXT[],
    is_active BOOLEAN DEFAULT TRUE,
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
);
```

### product_variants table (варианты товаров)
```sql
CREATE TABLE product_variants (
    id SERIAL PRIMARY KEY,
    variant_id UUID DEFAULT gen_random_uuid() UNIQUE NOT NULL,
    product_id UUID NOT NULL REFERENCES products(product_id) ON DELETE CASCADE,
    sku VARCHAR(100),                      -- Артикул (опционально)
    title VARCHAR(255) NOT NULL,           -- Название варианта
    price INTEGER NOT NULL,                -- Цена варианта
    currency VARCHAR(3) DEFAULT 'RUB',
    image_url TEXT,                        -- Картинка варианта
    description TEXT,                      -- Описание варианта
    stock_count INTEGER DEFAULT 0,         -- Остаток варианта
    attributes JSONB DEFAULT '{}',         -- Характеристики варианта
    is_active BOOLEAN DEFAULT TRUE,
    is_default BOOLEAN DEFAULT FALSE,      -- Вариант по умолчанию
    sort_order INTEGER DEFAULT 0,          -- Порядок сортировки
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
);
```

### Полезный VIEW
```sql
CREATE VIEW products_with_default_variant AS
SELECT 
    p.product_id,
    p.title as product_title,
    p.description as product_description,
    p.category,
    v.variant_id,
    v.title as variant_title,
    v.price,
    v.image_url,
    v.stock_count,
    v.attributes
FROM products p
LEFT JOIN LATERAL (
    SELECT * FROM product_variants
    WHERE product_id = p.product_id
    ORDER BY is_default DESC, sort_order ASC
    LIMIT 1
) v ON true;
```

## 🔄 Migration

### Шаг 1: Backup базы данных
```bash
pg_dump $DATABASE_URL > backup.sql
```

### Шаг 2: Запустить миграцию
```bash
psql $DATABASE_URL < sql/migrations/002_migrate_to_variants.sql
```

### Что делает миграция:
1. ✅ Переименовывает `products` → `products_old`
2. ✅ Создает новые таблицы `products` и `product_variants`
3. ✅ Мигрирует все данные (каждый старый товар → базовый товар + 1 вариант)
4. ✅ Создает индексы и триггеры
5. ✅ Сохраняет `products_old` как backup (опционально можно удалить)

### Шаг 3: Проверить миграцию
```sql
-- Проверить что все товары мигрировали
SELECT COUNT(*) FROM products;
SELECT COUNT(*) FROM product_variants;

-- Должно быть равное количество

-- Проверить конкретный товар
SELECT * FROM products WHERE shop_id = 1;
SELECT * FROM product_variants WHERE product_id = 'uuid-here';
```

## 📡 API Endpoints

### 1. GET /api/products/{shop_id} - Получить все товары

**Response:**
```json
[
  {
    "product_id": "uuid-1",
    "title": "iPhone 15 Pro",
    "description": "Latest iPhone",
    "category": "Electronics",
    "variants": [
      {
        "variant_id": "var-uuid-1",
        "title": "iPhone 15 Pro 128GB Black",
        "price": 100000,
        "currency": "RUB",
        "image_url": "iphone-128-black.jpg",
        "stock_count": 5,
        "attributes": {"storage": "128GB", "color": "Black"},
        "is_default": true
      },
      {
        "variant_id": "var-uuid-2",
        "title": "iPhone 15 Pro 256GB Black",
        "price": 120000,
        "currency": "RUB",
        "image_url": "iphone-256-black.jpg",
        "stock_count": 3,
        "attributes": {"storage": "256GB", "color": "Black"},
        "is_default": false
      }
    ]
  }
]
```

### 2. GET /api/products/detail/{product_id} - Детали товара

**Response:**
```json
{
  "product_id": "uuid-1",
  "title": "iPhone 15 Pro",
  "description": "Latest iPhone model with advanced features",
  "category": "Electronics",
  "tags": ["smartphone", "apple"],
  "variants": [
    {
      "variant_id": "var-uuid-1",
      "title": "iPhone 15 Pro 128GB Black",
      "price": 100000,
      "stock_count": 5,
      "image_url": "iphone-128-black.jpg",
      "description": "128GB storage, Black color",
      "attributes": {"storage": "128GB", "color": "Black"},
      "is_default": true
    },
    {
      "variant_id": "var-uuid-2",
      "title": "iPhone 15 Pro 256GB Black",
      "price": 120000,
      "stock_count": 3,
      "image_url": "iphone-256-black.jpg",
      "description": "256GB storage, Black color",
      "attributes": {"storage": "256GB", "color": "Black"}
    }
  ]
}
```

### 3. POST /api/admin/products - Создать товар с вариантами

**Request:**
```json
{
  "shop_id": 1,
  "title": "Nike Air Max",
  "description": "Running shoes",
  "category": "Shoes",
  "tags": ["shoes", "nike", "running"],
  "variants": [
    {
      "title": "Nike Air Max Size 41 Black",
      "price": 5000,
      "image_url": "nike-41-black.jpg",
      "stock_count": 10,
      "attributes": {"size": "41", "color": "Black"},
      "is_default": true
    },
    {
      "title": "Nike Air Max Size 42 Black",
      "price": 5000,
      "image_url": "nike-42-black.jpg",
      "stock_count": 5,
      "attributes": {"size": "42", "color": "Black"}
    },
    {
      "title": "Nike Air Max Size 41 White",
      "price": 5500,
      "image_url": "nike-41-white.jpg",
      "stock_count": 0,
      "attributes": {"size": "41", "color": "White"}
    }
  ]
}
```

**Response:**
```json
{
  "success": true,
  "product_id": "new-uuid",
  "variants_created": 3
}
```

### 4. POST /api/admin/products/{product_id}/variants - Добавить вариант

**Request:**
```json
{
  "title": "Nike Air Max Size 43 Black",
  "price": 5000,
  "image_url": "nike-43-black.jpg",
  "stock_count": 3,
  "attributes": {"size": "43", "color": "Black"}
}
```

**Response:**
```json
{
  "success": true,
  "variant_id": "new-variant-uuid"
}
```

### 5. PUT /api/admin/variants/{variant_id} - Обновить вариант

**Request:**
```json
{
  "price": 4500,
  "stock_count": 15,
  "image_url": "nike-41-black-new.jpg"
}
```

**Response:**
```json
{
  "success": true,
  "variant_id": "variant-uuid"
}
```

### 6. DELETE /api/admin/variants/{variant_id} - Удалить вариант

**Response:**
```json
{
  "success": true,
  "message": "Variant deleted"
}
```

## 💻 Flutter Implementation

### Updated Product Model

```dart
class Product {
  final String productId;
  final int shopId;
  final String title;
  final String? description;
  final String? category;
  final List<String>? tags;
  final bool isActive;
  final List<ProductVariant> variants;
  final DateTime createdAt;
  final DateTime updatedAt;
  
  Product({
    required this.productId,
    required this.shopId,
    required this.title,
    this.description,
    this.category,
    this.tags,
    required this.isActive,
    required this.variants,
    required this.createdAt,
    required this.updatedAt,
  });
  
  // Get default variant for display in lists
  ProductVariant? get defaultVariant {
    try {
      return variants.firstWhere((v) => v.isDefault);
    } catch (e) {
      return variants.isNotEmpty ? variants.first : null;
    }
  }
  
  // Get cheapest variant
  ProductVariant? get cheapestVariant {
    if (variants.isEmpty) return null;
    return variants.reduce((a, b) => a.price < b.price ? a : b);
  }
  
  // Check if any variant is in stock
  bool get hasStock {
    return variants.any((v) => v.stockCount > 0);
  }
  
  factory Product.fromJson(Map<String, dynamic> json) {
    return Product(
      productId: json['product_id'],
      shopId: json['shop_id'],
      title: json['title'],
      description: json['description'],
      category: json['category'],
      tags: json['tags'] != null ? List<String>.from(json['tags']) : null,
      isActive: json['is_active'] ?? true,
      variants: (json['variants'] as List<dynamic>?)
          ?.map((v) => ProductVariant.fromJson(v))
          .toList() ?? [],
      createdAt: DateTime.parse(json['created_at']),
      updatedAt: DateTime.parse(json['updated_at']),
    );
  }
}
```

### ProductVariant Model

```dart
class ProductVariant {
  final String variantId;
  final String productId;
  final String? sku;
  final String title;
  final int price; // в копейках
  final String currency;
  final String? imageUrl;
  final String? description;
  final int stockCount;
  final Map<String, dynamic> attributes;
  final bool isActive;
  final bool isDefault;
  final int sortOrder;
  final DateTime createdAt;
  final DateTime updatedAt;
  
  ProductVariant({
    required this.variantId,
    required this.productId,
    this.sku,
    required this.title,
    required this.price,
    required this.currency,
    this.imageUrl,
    this.description,
    required this.stockCount,
    required this.attributes,
    required this.isActive,
    required this.isDefault,
    required this.sortOrder,
    required this.createdAt,
    required this.updatedAt,
  });
  
  // Formatted price
  String get formattedPrice {
    return '${(price / 100).toStringAsFixed(2)} $currency';
  }
  
  // Check if in stock
  bool get isInStock => stockCount > 0;
  
  // Format attributes for display
  String get formattedAttributes {
    if (attributes.isEmpty) return '';
    return attributes.entries
        .map((e) => '${e.key}: ${e.value}')
        .join(', ');
  }
  
  factory ProductVariant.fromJson(Map<String, dynamic> json) {
    return ProductVariant(
      variantId: json['variant_id'],
      productId: json['product_id'],
      sku: json['sku'],
      title: json['title'],
      price: json['price'],
      currency: json['currency'] ?? 'RUB',
      imageUrl: json['image_url'],
      description: json['description'],
      stockCount: json['stock_count'] ?? 0,
      attributes: json['attributes'] ?? {},
      isActive: json['is_active'] ?? true,
      isDefault: json['is_default'] ?? false,
      sortOrder: json['sort_order'] ?? 0,
      createdAt: DateTime.parse(json['created_at']),
      updatedAt: DateTime.parse(json['updated_at']),
    );
  }
  
  Map<String, dynamic> toJson() {
    return {
      'variant_id': variantId,
      'product_id': productId,
      'sku': sku,
      'title': title,
      'price': price,
      'currency': currency,
      'image_url': imageUrl,
      'description': description,
      'stock_count': stockCount,
      'attributes': attributes,
      'is_active': isActive,
      'is_default': isDefault,
      'sort_order': sortOrder,
    };
  }
}
```

### UI: Product Card with Variants

```dart
class ProductCard extends StatelessWidget {
  final Product product;
  final VoidCallback? onTap;
  
  const ProductCard({
    Key? key,
    required this.product,
    this.onTap,
  }) : super(key: key);
  
  @override
  Widget build(BuildContext context) {
    final defaultVariant = product.defaultVariant;
    
    if (defaultVariant == null) {
      return Card(
        child: ListTile(
          title: Text(product.title),
          subtitle: Text('No variants available'),
        ),
      );
    }
    
    return Card(
      child: InkWell(
        onTap: onTap,
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            // Product image
            if (defaultVariant.imageUrl != null)
              AspectRatio(
                aspectRatio: 1,
                child: Image.network(
                  defaultVariant.imageUrl!,
                  fit: BoxFit.cover,
                  errorBuilder: (context, error, stackTrace) {
                    return Container(
                      color: Colors.grey[300],
                      child: Icon(Icons.image, size: 50),
                    );
                  },
                ),
              ),
            
            Padding(
              padding: EdgeInsets.all(12),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  // Product title
                  Text(
                    product.title,
                    style: TextStyle(
                      fontSize: 16,
                      fontWeight: FontWeight.bold,
                    ),
                    maxLines: 2,
                    overflow: TextOverflow.ellipsis,
                  ),
                  SizedBox(height: 4),
                  
                  // Variant title (if different from product)
                  if (defaultVariant.title != product.title)
                    Text(
                      defaultVariant.title,
                      style: TextStyle(
                        fontSize: 12,
                        color: Colors.grey[600],
                      ),
                      maxLines: 1,
                      overflow: TextOverflow.ellipsis,
                    ),
                  
                  SizedBox(height: 8),
                  
                  // Price
                  Text(
                    defaultVariant.formattedPrice,
                    style: TextStyle(
                      fontSize: 18,
                      fontWeight: FontWeight.bold,
                      color: Colors.green,
                    ),
                  ),
                  
                  SizedBox(height: 4),
                  
                  // Stock status
                  Row(
                    children: [
                      Icon(
                        defaultVariant.isInStock ? Icons.check_circle : Icons.cancel,
                        size: 16,
                        color: defaultVariant.isInStock ? Colors.green : Colors.red,
                      ),
                      SizedBox(width: 4),
                      Text(
                        defaultVariant.isInStock 
                            ? 'В наличии: ${defaultVariant.stockCount}'
                            : 'Нет в наличии',
                        style: TextStyle(
                          fontSize: 12,
                          color: defaultVariant.isInStock ? Colors.green : Colors.red,
                        ),
                      ),
                    ],
                  ),
                  
                  // Number of variants
                  if (product.variants.length > 1)
                    Padding(
                      padding: EdgeInsets.only(top: 8),
                      child: Text(
                        '${product.variants.length} вариантов',
                        style: TextStyle(
                          fontSize: 12,
                          color: Colors.blue,
                          fontWeight: FontWeight.w500,
                        ),
                      ),
                    ),
                ],
              ),
            ),
          ],
        ),
      ),
    );
  }
}
```

### UI: Variant Selector

```dart
class VariantSelector extends StatefulWidget {
  final Product product;
  final Function(ProductVariant) onVariantSelected;
  
  const VariantSelector({
    Key? key,
    required this.product,
    required this.onVariantSelected,
  }) : super(key: key);
  
  @override
  _VariantSelectorState createState() => _VariantSelectorState();
}

class _VariantSelectorState extends State<VariantSelector> {
  late ProductVariant selectedVariant;
  
  @override
  void initState() {
    super.initState();
    selectedVariant = widget.product.defaultVariant ?? widget.product.variants.first;
  }
  
  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text(
          'Выберите вариант:',
          style: TextStyle(fontSize: 16, fontWeight: FontWeight.bold),
        ),
        SizedBox(height: 12),
        
        // Variant chips/buttons
        Wrap(
          spacing: 8,
          runSpacing: 8,
          children: widget.product.variants.map((variant) {
            final isSelected = variant.variantId == selectedVariant.variantId;
            final isAvailable = variant.isInStock;
            
            return ChoiceChip(
              label: Column(
                mainAxisSize: MainAxisSize.min,
                children: [
                  Text(variant.formattedAttributes),
                  if (!isAvailable)
                    Text(
                      'Нет в наличии',
                      style: TextStyle(fontSize: 10, color: Colors.red),
                    ),
                ],
              ),
              selected: isSelected,
              onSelected: isAvailable ? (selected) {
                if (selected) {
                  setState(() {
                    selectedVariant = variant;
                  });
                  widget.onVariantSelected(variant);
                }
              } : null,
              backgroundColor: isAvailable ? null : Colors.grey[300],
            );
          }).toList(),
        ),
        
        SizedBox(height: 16),
        
        // Selected variant details
        Card(
          color: Colors.blue[50],
          child: Padding(
            padding: EdgeInsets.all(16),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(
                  selectedVariant.title,
                  style: TextStyle(fontSize: 16, fontWeight: FontWeight.bold),
                ),
                SizedBox(height: 8),
                Text(
                  selectedVariant.formattedPrice,
                  style: TextStyle(fontSize: 20, color: Colors.green, fontWeight: FontWeight.bold),
                ),
                SizedBox(height: 4),
                Text(
                  'В наличии: ${selectedVariant.stockCount} шт.',
                  style: TextStyle(fontSize: 14),
                ),
                if (selectedVariant.description != null)
                  Padding(
                    padding: EdgeInsets.only(top: 8),
                    child: Text(selectedVariant.description!),
                  ),
              ],
            ),
          ),
        ),
      ],
    );
  }
}
```

## 🎯 Real-World Examples

### Example 1: Shoe Store

```json
{
  "title": "Nike Air Max 2024",
  "description": "Premium running shoes",
  "category": "Shoes",
  "variants": [
    {
      "title": "Nike Air Max 2024 Size 41 Black",
      "price": 8000,
      "stock_count": 5,
      "image_url": "nike-41-black.jpg",
      "attributes": {"size": "41", "color": "Black"}
    },
    {
      "title": "Nike Air Max 2024 Size 42 Black",
      "price": 8000,
      "stock_count": 3,
      "image_url": "nike-42-black.jpg",
      "attributes": {"size": "42", "color": "Black"}
    },
    {
      "title": "Nike Air Max 2024 Size 41 White",
      "price": 8500,
      "stock_count": 0,
      "image_url": "nike-41-white.jpg",
      "attributes": {"size": "41", "color": "White"}
    }
  ]
}
```

### Example 2: Electronics Store

```json
{
  "title": "iPhone 15 Pro",
  "description": "Latest iPhone with A17 Pro chip",
  "category": "Smartphones",
  "variants": [
    {
      "title": "iPhone 15 Pro 128GB Natural Titanium",
      "price": 100000,
      "stock_count": 10,
      "image_url": "iphone-128-titanium.jpg",
      "description": "128GB storage, Natural Titanium color",
      "attributes": {"storage": "128GB", "color": "Natural Titanium"}
    },
    {
      "title": "iPhone 15 Pro 256GB Natural Titanium",
      "price": 120000,
      "stock_count": 5,
      "image_url": "iphone-256-titanium.jpg",
      "description": "256GB storage, Natural Titanium color",
      "attributes": {"storage": "256GB", "color": "Natural Titanium"}
    },
    {
      "title": "iPhone 15 Pro 512GB Natural Titanium",
      "price": 140000,
      "stock_count": 2,
      "image_url": "iphone-512-titanium.jpg",
      "description": "512GB storage, Natural Titanium color",
      "attributes": {"storage": "512GB", "color": "Natural Titanium"}
    },
    {
      "title": "iPhone 15 Pro 128GB Blue Titanium",
      "price": 100000,
      "stock_count": 8,
      "image_url": "iphone-128-blue.jpg",
      "description": "128GB storage, Blue Titanium color",
      "attributes": {"storage": "128GB", "color": "Blue Titanium"}
    }
  ]
}
```

### Example 3: Clothing Store

```json
{
  "title": "Supreme Box Logo Hoodie",
  "description": "Classic Supreme hoodie",
  "category": "Clothing",
  "variants": [
    {
      "title": "Supreme Hoodie S Red",
      "price": 15000,
      "stock_count": 2,
      "image_url": "supreme-s-red.jpg",
      "attributes": {"size": "S", "color": "Red"}
    },
    {
      "title": "Supreme Hoodie M Red",
      "price": 15000,
      "stock_count": 5,
      "image_url": "supreme-m-red.jpg",
      "attributes": {"size": "M", "color": "Red"}
    },
    {
      "title": "Supreme Hoodie L Red",
      "price": 15000,
      "stock_count": 3,
      "image_url": "supreme-l-red.jpg",
      "attributes": {"size": "L", "color": "Red"}
    },
    {
      "title": "Supreme Hoodie M Black",
      "price": 16000,
      "stock_count": 4,
      "image_url": "supreme-m-black.jpg",
      "attributes": {"size": "M", "color": "Black"}
    }
  ]
}
```

## ✅ Benefits

### For Shop Owners
- ✅ Разные цены для разных размеров/цветов/моделей
- ✅ Отдельный учет остатков по каждому варианту
- ✅ Разные картинки для каждого варианта
- ✅ Гибкость в управлении товарами

### For Customers
- ✅ Видят все доступные варианты
- ✅ Знают точные остатки по каждому варианту
- ✅ Видят правильные цены и картинки
- ✅ Не заказывают товары, которых нет в наличии

### For Developers
- ✅ Полная гибкость в реализации UI
- ✅ Легко добавлять новые типы характеристик
- ✅ Масштабируемая архитектура
- ✅ Готовые модели данных

## 🚀 Deployment Checklist

- [ ] Backup базы данных
- [ ] Запустить миграцию `002_migrate_to_variants.sql`
- [ ] Проверить что все товары мигрировали
- [ ] Обновить API бэкенд (ProductController)
- [ ] Обновить модели данных во Flutter приложении
- [ ] Обновить UI для отображения вариантов
- [ ] Протестировать создание товаров с вариантами
- [ ] Протестировать корзину с вариантами
- [ ] Протестировать заказы с вариантами
- [ ] Задеплоить на production

## 📚 Additional Resources

- `sql/schema_variants.sql` - Полная схема БД
- `sql/migrations/002_migrate_to_variants.sql` - Скрипт миграции
- `API_ENDPOINTS.md` - API reference (будет обновлен)
- `FLUTTER_ADMIN_GUIDE.md` - Flutter guide (будет обновлен)

---

**Готово к использованию!** 🎉
