# Product Attributes Feature - Flutter Implementation Guide

## Overview

Starting from version 2.0, products now support flexible `attributes` field (JSONB) for storing product variants and characteristics.

## Updated Product Model

```dart
class Product {
  final int id;
  final String productId; // UUID
  final int shopId;
  final String title;
  final int price; // цена в копейках (100 = 1.00 RUB)
  final String currency;
  final String? imageUrl;
  final String? description;
  final int stockCount;
  final bool isActive;
  final String? category;
  final List<String>? tags;
  final Map<String, dynamic>? attributes; // NEW! Flexible attributes
  final DateTime createdAt;
  final DateTime updatedAt;

  Product({
    required this.id,
    required this.productId,
    required this.shopId,
    required this.title,
    required this.price,
    required this.currency,
    this.imageUrl,
    this.description,
    required this.stockCount,
    required this.isActive,
    this.category,
    this.tags,
    this.attributes, // NEW!
    required this.createdAt,
    required this.updatedAt,
  });

  // Удобные геттеры
  String get formattedPrice {
    final rubles = price / 100;
    return '${rubles.toStringAsFixed(2)} $currency';
  }

  bool get isInStock => stockCount > 0;
  
  bool get isAvailable => isActive && isInStock;
  
  // NEW! Check if product has variants
  bool get hasAttributes => attributes != null && attributes!.isNotEmpty;
  
  // NEW! Get formatted attributes string
  String get formattedAttributes {
    if (attributes == null || attributes!.isEmpty) return '';
    return attributes!.entries
        .map((e) => '${e.key}: ${e.value}')
        .join(', ');
  }

  factory Product.fromJson(Map<String, dynamic> json) {
    return Product(
      id: json['id'],
      productId: json['product_id'],
      shopId: json['shop_id'],
      title: json['title'],
      price: json['price'],
      currency: json['currency'] ?? 'RUB',
      imageUrl: json['image_url'],
      description: json['description'],
      stockCount: json['stock_count'] ?? 0,
      isActive: json['is_active'] ?? true,
      category: json['category'],
      tags: json['tags'] != null ? List<String>.from(json['tags']) : null,
      attributes: json['attributes'], // Parse JSONB field
      createdAt: DateTime.parse(json['created_at']),
      updated At: DateTime.parse(json['updated_at']),
    );
  }

  Map<String, dynamic> toJson() {
    return {
      'id': id,
      'product_id': productId,
      'shop_id': shopId,
      'title': title,
      'price': price,
      'currency': currency,
      'image_url': imageUrl,
      'description': description,
      'stock_count': stockCount,
      'is_active': isActive,
      'category': category,
      'tags': tags,
      'attributes': attributes, // Include attributes in JSON
      'created_at': createdAt.toIso8601String(),
      'updated_at': updatedAt.toIso8601String(),
    };
  }
}
```

## Usage Examples

### 1. Create Product with Attributes

```dart
// Shoes with size and color
await productsProvider.createProduct(
  shopId: 1,
  title: "Nike Air Max",
  price: 5000 * 100, // 5000 rubles in kopecks
  attributes: {
    "size": "42",
    "color": "black",
    "gender": "unisex"
  },
);

// iPhone with storage and model
await productsProvider.createProduct(
  shopId: 1,
  title: "iPhone 15 Pro Max",
  price: 120000 * 100,
  attributes: {
    "storage": "256GB",
    "color": "Natural Titanium",
    "model": "Pro Max"
  },
);

// Simple product without variants
await productsProvider.createProduct(
  shopId: 1,
  title: "Coffee Beans",
  price: 800 * 100,
  // no attributes specified
);
```

### 2. Display Attributes in UI

```dart
class ProductCard extends StatelessWidget {
  final Product product;
  
  @override
  Widget build(BuildContext context) {
    return Card(
      child: ListTile(
        title: Text(product.title),
        subtitle: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text(product.formattedPrice),
            if (product.hasAttributes)
              Text(
                product.formattedAttributes,
                style: TextStyle(
                  fontSize: 12,
                  color: Colors.grey[600],
                ),
              ),
          ],
        ),
      ),
    );
  }
}
```

### 3. ProductFormScreen with Attributes

```dart
class ProductFormScreen extends StatefulWidget {
  final Product? product;
  
  @override
  State<ProductFormScreen> createState() => _ProductFormScreenState();
}

class _ProductFormScreenState extends State<ProductFormScreen> {
  final _formKey = GlobalKey<FormState>();
  final _titleController = TextEditingController();
  final _priceController = TextEditingController();
  
  // Attribute controllers
  final List<AttributeController> _attributeControllers = [];
  
  @override
  void initState() {
    super.initState();
    if (widget.product?.attributes != null) {
      // Load existing attributes
      widget.product!.attributes!.forEach((key, value) {
        _attributeControllers.add(
          AttributeController(
            keyController: TextEditingController(text: key),
            valueController: TextEditingController(text: value.toString()),
          ),
        );
      });
    }
  }
  
  void _addAttribute() {
    setState(() {
      _attributeControllers.add(
        AttributeController(
          keyController: TextEditingController(),
          valueController: TextEditingController(),
        ),
      );
    });
  }
  
  void _removeAttribute(int index) {
    setState(() {
      _attributeControllers[index].dispose();
      _attributeControllers.removeAt(index);
    });
  }
  
  Map<String, dynamic>? _getAttributes() {
    if (_attributeControllers.isEmpty) return null;
    
    final Map<String, dynamic> attributes = {};
    for (final controller in _attributeControllers) {
      final key = controller.keyController.text.trim();
      final value = controller.valueController.text.trim();
      if (key.isNotEmpty && value.isNotEmpty) {
        attributes[key] = value;
      }
    }
    return attributes.isEmpty ? null : attributes;
  }
  
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(widget.product == null ? 'Добавить товар' : 'Редактировать'),
      ),
      body: Form(
        key: _formKey,
        child: ListView(
          padding: EdgeInsets.all(16),
          children: [
            // Title field
            TextFormField(
              controller: _titleController,
              decoration: InputDecoration(labelText: 'Название'),
              validator: (v) => v?.isEmpty == true ? 'Обязательно' : null,
            ),
            SizedBox(height: 16),
            
            // Price field
            TextFormField(
              controller: _priceController,
              decoration: InputDecoration(labelText: 'Цена (рублей)'),
              keyboardType: TextInputType.number,
              validator: (v) => v?.isEmpty == true ? 'Обязательно' : null,
            ),
            SizedBox(height: 24),
            
            // Attributes section
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceBetween,
              children: [
                Text(
                  'Характеристики',
                  style: Theme.of(context).textTheme.titleMedium,
                ),
                IconButton(
                  icon: Icon(Icons.add),
                  onPressed: _addAttribute,
                ),
              ],
            ),
            SizedBox(height: 8),
            
            // Attributes list
            ..._attributeControllers.asMap().entries.map((entry) {
              final index = entry.key;
              final controller = entry.value;
              return Card(
                child: Padding(
                  padding: EdgeInsets.all(8),
                  child: Row(
                    children: [
                      Expanded(
                        child: TextFormField(
                          controller: controller.keyController,
                          decoration: InputDecoration(
                            labelText: 'Название',
                            hintText: 'size, color, storage',
                          ),
                        ),
                      ),
                      SizedBox(width: 8),
                      Expanded(
                        child: TextFormField(
                          controller: controller.valueController,
                          decoration: InputDecoration(
                            labelText: 'Значение',
                            hintText: '42, black, 256GB',
                          ),
                        ),
                      ),
                      IconButton(
                        icon: Icon(Icons.delete, color: Colors.red),
                        onPressed: () => _removeAttribute(index),
                      ),
                    ],
                  ),
                ),
              );
            }).toList(),
            
            SizedBox(height: 24),
            
            // Save button
            ElevatedButton(
              onPressed: _saveProduct,
              child: Text('Сохранить'),
            ),
          ],
        ),
      ),
    );
  }
  
  Future<void> _saveProduct() async {
    if (!_formKey.currentState!.validate()) return;
    
    final attributes = _getAttributes();
    final price = (double.parse(_priceController.text) * 100).toInt();
    
    try {
      if (widget.product == null) {
        // Create new product
        await context.read<ProductsProvider>().createProduct(
          shopId: context.read<ShopProvider>().shop!.id,
          title: _titleController.text,
          price: price,
          attributes: attributes,
        );
      } else {
        // Update existing product
        await context.read<ProductsProvider>().updateProduct(
          productId: widget.product!.productId,
          title: _titleController.text,
          price: price,
          attributes: attributes,
        );
      }
      
      Navigator.pop(context);
    } catch (e) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text('Ошибка: $e')),
      );
    }
  }
}

class AttributeController {
  final TextEditingController keyController;
  final TextEditingController valueController;
  
  AttributeController({
    required this.keyController,
    required this.valueController,
  });
  
  void dispose() {
    keyController.dispose();
    valueController.dispose();
  }
}
```

### 4. Update ProductsProvider

```dart
class ProductsProvider with ChangeNotifier {
  final ApiService _apiService = ApiService();
  
  // ... existing code ...
  
  Future<void> createProduct({
    required int shopId,
    required String title,
    required int price,
    String? imageUrl,
    String? description,
    int stockCount = 0,
    bool isActive = true,
    String? category,
    List<String>? tags,
    Map<String, dynamic>? attributes, // NEW!
  }) async {
    _isLoading = true;
    _error = null;
    notifyListeners();

    try {
      await _apiService.createProduct(
        shopId: shopId,
        title: title,
        price: price,
        imageUrl: imageUrl,
        description: description,
        stockCount: stockCount,
        isActive: isActive,
        category: category,
        tags: tags,
        attributes: attributes, // Pass to API
      );

      // Reload products
      await loadProducts(shopId);
      
      _isLoading = false;
      notifyListeners();
    } catch (e) {
      _error = e.toString();
      _isLoading = false;
      notifyListeners();
      rethrow;
    }
  }

  Future<void> updateProduct({
    required String productId,
    String? title,
    int? price,
    String? imageUrl,
    String? description,
    int? stockCount,
    bool? isActive,
    String? category,
    List<String>? tags,
    Map<String, dynamic>? attributes, // NEW!
  }) async {
    _isLoading = true;
    _error = null;
    notifyListeners();

    try {
      await _apiService.updateProduct(
        productId: productId,
        title: title,
        price: price,
        imageUrl: imageUrl,
        description: description,
        stockCount: stockCount,
        isActive: isActive,
        category: category,
        tags: tags,
        attributes: attributes, // Pass to API
      );

      // Reload products
      final shopId = _products.firstWhere((p) => p.productId == productId).shopId;
      await loadProducts(shopId);
      
      _isLoading = false;
      notifyListeners();
    } catch (e) {
      _error = e.toString();
      _isLoading = false;
      notifyListeners();
      rethrow;
    }
  }
}
```

### 5. Update ApiService

```dart
class ApiService {
  static const String baseUrl = 'https://shopkit-backend.onrender.com/api';
  
  Future<void> createProduct({
    required int shopId,
    required String title,
    required int price,
    String? imageUrl,
    String? description,
    int stockCount = 0,
    bool isActive = true,
    String? category,
    List<String>? tags,
    Map<String, dynamic>? attributes, // NEW!
  }) async {
    final body = {
      'shop_id': shopId,
      'title': title,
      'price': price,
      'currency': 'RUB',
      if (imageUrl != null) 'image_url': imageUrl,
      if (description != null) 'description': description,
      'stock_count': stockCount,
      'is_active': isActive,
      if (category != null) 'category': category,
      if (tags != null) 'tags': tags,
      if (attributes != null) 'attributes': attributes, // Include attributes
    };

    final response = await http.post(
      Uri.parse('$baseUrl/admin/products'),
      headers: {
        'Content-Type': 'application/json',
        'X-Owner-TG-ID': await AuthService().getOwnerTgId() ?? '',
      },
      body: jsonEncode(body),
    );

    if (response.statusCode != 201) {
      throw ApiException(
        'Failed to create product: ${response.body}',
        response.statusCode,
      );
    }
  }

  Future<void> updateProduct({
    required String productId,
    String? title,
    int? price,
    String? imageUrl,
    String? description,
    int? stockCount,
    bool? isActive,
    String? category,
    List<String>? tags,
    Map<String, dynamic>? attributes, // NEW!
  }) async {
    final body = <String, dynamic>{};
    
    if (title != null) body['title'] = title;
    if (price != null) body['price'] = price;
    if (imageUrl != null) body['image_url'] = imageUrl;
    if (description != null) body['description'] = description;
    if (stockCount != null) body['stock_count'] = stockCount;
    if (isActive != null) body['is_active'] = isActive;
    if (category != null) body['category'] = category;
    if (tags != null) body['tags'] = tags;
    if (attributes != null) body['attributes'] = attributes; // Include attributes

    final response = await http.put(
      Uri.parse('$baseUrl/admin/products/$productId'),
      headers: {
        'Content-Type': 'application/json',
        'X-Owner-TG-ID': await AuthService().getOwnerTgId() ?? '',
      },
      body: jsonEncode(body),
    );

    if (response.statusCode != 200) {
      throw ApiException(
        'Failed to update product: ${response.body}',
        response.statusCode,
      );
    }
  }
}
```

## Real-World Examples

### Example 1: Clothing Store

```dart
// T-Shirt with size and color
await productsProvider.createProduct(
  shopId: 1,
  title: "Premium Cotton T-Shirt",
  price: 1500 * 100,
  category: "Clothing",
  attributes: {
    "size": "L",
    "color": "Navy Blue",
    "material": "100% Cotton",
    "fit": "Regular"
  },
);
```

### Example 2: Electronics Store

```dart
// Laptop with specs
await productsProvider.createProduct(
  shopId: 1,
  title: "MacBook Pro 14\"",
  price: 180000 * 100,
  category: "Electronics",
  attributes: {
    "processor": "M3 Pro",
    "ram": "18GB",
    "storage": "512GB SSD",
    "screen": "14.2\" Liquid Retina XDR",
    "color": "Space Black"
  },
);
```

### Example 3: Shoe Store

```dart
// Running shoes with size options
for (int size = 38; size <= 45; size++) {
  await productsProvider.createProduct(
    shopId: 1,
    title: "Nike Air Zoom Pegasus 40",
    price: 8500 * 100,
    stockCount: 5,
    category: "Shoes",
    attributes: {
      "size": size.toString(),
      "color": "Black/White",
      "type": "Running",
      "gender": "Unisex"
    },
  );
}
```

## Benefits

✅ **Universal** - works for any product type  
✅ **Flexible** - add any custom fields  
✅ **Optional** - simple products don't need attributes  
✅ **Type-safe** - `Map<String, dynamic>` in Dart  
✅ **Searchable** - indexed in PostgreSQL with GIN  

## Migration Notes

- Existing products will have `attributes: null`
- No breaking changes - fully backward compatible
- Apps can continue without using attributes
- Gradually add attributes to products as needed
