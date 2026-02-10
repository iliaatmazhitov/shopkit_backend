# 📱 Flutter Guide - Android & iOS App Development

> **Production API:** `https://shopkit-backend.onrender.com/api`  
> **Status:** ✅ Production-ready

Полное руководство по разработке e-commerce приложений на Flutter для Android и iOS с интеграцией ShopKit backend.

## Содержание

1. [Quick Start](#quick-start)
2. [Настройка проекта](#настройка-проекта)
3. [Структура проекта](#структура-проекта)
4. [Модели данных](#модели-данных)
5. [API клиент](#api-клиент)
6. [State Management](#state-management)
7. [UI компоненты](#ui-компоненты)
8. [Навигация](#навигация)
9. [Платформо-специфичные фичи](#платформо-специфичные-фичи)
10. [Тестирование](#тестирование)
11. [Деплой](#деплой)
12. [Best Practices](#best-practices)

---

## Quick Start

### Требования

- **Flutter SDK:** 3.16.0 или выше
- **Dart:** 3.2.0 или выше
- **Android Studio:** для Android разработки
- **Xcode:** для iOS разработки (только на macOS)

### Установка Flutter

```bash
# macOS
brew install flutter

# Linux
snap install flutter --classic

# Windows
# Скачать с https://docs.flutter.dev/get-started/install

# Проверка установки
flutter doctor
```

### Создание проекта

```bash
# Создать новый проект
flutter create shopkit_app
cd shopkit_app

# Добавить зависимости
flutter pub add http provider shared_preferences
flutter pub add --dev mockito build_runner

# Запустить на эмуляторе
flutter run
```

---

## Настройка проекта

### pubspec.yaml

```yaml
name: shopkit_app
description: E-commerce app with ShopKit backend
publish_to: 'none'
version: 1.0.0+1

environment:
  sdk: '>=3.2.0 <4.0.0'

dependencies:
  flutter:
    sdk: flutter
  
  # HTTP клиент
  http: ^1.1.0
  
  # State management
  provider: ^6.1.1
  
  # Локальное хранилище
  shared_preferences: ^2.2.2
  
  # UI
  cached_network_image: ^3.3.0
  
  # Утилиты
  intl: ^0.18.1
  uuid: ^4.2.2

dev_dependencies:
  flutter_test:
    sdk: flutter
  flutter_lints: ^3.0.0
  mockito: ^5.4.4
  build_runner: ^2.4.7

flutter:
  uses-material-design: true
```

### Конфигурация API

Создайте `lib/config/api_config.dart`:

```dart
class ApiConfig {
  static const String baseUrl = 'https://shopkit-backend.onrender.com/api';
  
  // Timeouts
  static const Duration connectionTimeout = Duration(seconds: 10);
  static const Duration receiveTimeout = Duration(seconds: 30);
  
  // Retry configuration
  static const int maxRetries = 3;
  static const Duration retryDelay = Duration(seconds: 1);
}
```

---

## Структура проекта

```
lib/
├── config/
│   └── api_config.dart           # API конфигурация
├── models/
│   ├── shop.dart                 # Shop модель
│   ├── product.dart              # Product модель
│   ├── cart_item.dart            # CartItem модель
│   └── order.dart                # Order модель
├── services/
│   ├── api_service.dart          # API клиент
│   └── storage_service.dart      # Локальное хранилище
├── providers/
│   ├── shop_provider.dart        # Shop state
│   ├── cart_provider.dart        # Cart state
│   └── order_provider.dart       # Order state
├── screens/
│   ├── shop_screen.dart          # Экран магазина
│   ├── product_detail_screen.dart # Детали товара
│   ├── cart_screen.dart          # Корзина
│   ├── checkout_screen.dart      # Оформление заказа
│   └── orders_screen.dart        # История заказов
├── widgets/
│   ├── product_card.dart         # Карточка товара
│   ├── cart_item_widget.dart     # Элемент корзины
│   └── loading_indicator.dart    # Индикатор загрузки
└── main.dart                     # Entry point
```

---

## Модели данных

### Shop Model

`lib/models/shop.dart`:

```dart
class Shop {
  final int id;
  final String shopToken;
  final int ownerTgId;
  final String title;
  final String description;
  final String currency;
  final DateTime createdAt;
  final DateTime updatedAt;
  final int? productsCount;

  Shop({
    required this.id,
    required this.shopToken,
    required this.ownerTgId,
    required this.title,
    required this.description,
    required this.currency,
    required this.createdAt,
    required this.updatedAt,
    this.productsCount,
  });

  factory Shop.fromJson(Map<String, dynamic> json) {
    return Shop(
      id: json['id'] as int,
      shopToken: json['shop_token'] as String,
      ownerTgId: json['owner_tg_id'] as int,
      title: json['title'] as String,
      description: json['description'] as String? ?? '',
      currency: json['currency'] as String,
      createdAt: DateTime.parse(json['created_at'] as String),
      updatedAt: DateTime.parse(json['updated_at'] as String),
      productsCount: json['products_count'] as int?,
    );
  }

  Map<String, dynamic> toJson() {
    return {
      'id': id,
      'shop_token': shopToken,
      'owner_tg_id': ownerTgId,
      'title': title,
      'description': description,
      'currency': currency,
      'created_at': createdAt.toIso8601String(),
      'updated_at': updatedAt.toIso8601String(),
      if (productsCount != null) 'products_count': productsCount,
    };
  }
}
```

### Product Model

`lib/models/product.dart`:

```dart
class Product {
  final int id;
  final String productId; // UUID
  final int shopId;
  final String title;
  final int price; // В копейках/центах
  final String currency;
  final String? imageUrl;
  final String? description;
  final int stockCount;
  final bool isActive;
  final String? category;
  final List<String>? tags;
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
    required this.createdAt,
    required this.updatedAt,
  });

  // Цена в удобочитаемом формате
  String get formattedPrice {
    final priceInUnits = price / 100;
    return '${priceInUnits.toStringAsFixed(2)} $currency';
  }

  // Доступен ли товар
  bool get isAvailable => isActive && stockCount > 0;

  factory Product.fromJson(Map<String, dynamic> json) {
    return Product(
      id: json['id'] as int? ?? 0,
      productId: json['product_id'] as String,
      shopId: json['shop_id'] as int? ?? 0,
      title: json['title'] as String,
      price: json['price'] as int,
      currency: json['currency'] as String,
      imageUrl: json['image_url'] as String?,
      description: json['description'] as String?,
      stockCount: json['stock_count'] as int? ?? 0,
      isActive: json['is_active'] as bool? ?? true,
      category: json['category'] as String?,
      tags: (json['tags'] as List<dynamic>?)?.cast<String>(),
      createdAt: DateTime.parse(
        json['created_at'] as String? ?? DateTime.now().toIso8601String(),
      ),
      updatedAt: DateTime.parse(
        json['updated_at'] as String? ?? DateTime.now().toIso8601String(),
      ),
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
      'created_at': createdAt.toIso8601String(),
      'updated_at': updatedAt.toIso8601String(),
    };
  }
}
```

### CartItem Model

`lib/models/cart_item.dart`:

```dart
class CartItem {
  final String productId;
  final String title;
  int quantity;
  final int price;

  CartItem({
    required this.productId,
    required this.title,
    required this.quantity,
    required this.price,
  });

  // Общая стоимость позиции
  int get totalPrice => price * quantity;

  factory CartItem.fromJson(Map<String, dynamic> json) {
    return CartItem(
      productId: json['product_id'] as String,
      title: json['title'] as String,
      quantity: json['quantity'] as int,
      price: json['price'] as int,
    );
  }

  Map<String, dynamic> toJson() {
    return {
      'product_id': productId,
      'title': title,
      'quantity': quantity,
      'price': price,
    };
  }

  // Создать CartItem из Product
  factory CartItem.fromProduct(Product product, {int quantity = 1}) {
    return CartItem(
      productId: product.productId,
      title: product.title,
      quantity: quantity,
      price: product.price,
    );
  }

  CartItem copyWith({
    String? productId,
    String? title,
    int? quantity,
    int? price,
  }) {
    return CartItem(
      productId: productId ?? this.productId,
      title: title ?? this.title,
      quantity: quantity ?? this.quantity,
      price: price ?? this.price,
    );
  }
}
```

### Order Model

`lib/models/order.dart`:

```dart
enum OrderStatus {
  pending,
  completed,
  cancelled;

  static OrderStatus fromString(String status) {
    return OrderStatus.values.firstWhere(
      (e) => e.name == status,
      orElse: () => OrderStatus.pending,
    );
  }
}

class Order {
  final int id;
  final String orderId; // UUID
  final int shopId;
  final String? shopTitle;
  final String? shopToken;
  final int telegramUserId;
  final String customerName;
  final String? customerPhone;
  final List<CartItem> items;
  final int totalPrice;
  final String currency;
  final OrderStatus status;
  final String? paymentMethod;
  final String? notes;
  final DateTime createdAt;
  final DateTime updatedAt;

  Order({
    required this.id,
    required this.orderId,
    required this.shopId,
    this.shopTitle,
    this.shopToken,
    required this.telegramUserId,
    required this.customerName,
    this.customerPhone,
    required this.items,
    required this.totalPrice,
    required this.currency,
    required this.status,
    this.paymentMethod,
    this.notes,
    required this.createdAt,
    required this.updatedAt,
  });

  factory Order.fromJson(Map<String, dynamic> json) {
    return Order(
      id: json['id'] as int? ?? 0,
      orderId: json['order_id'] as String,
      shopId: json['shop_id'] as int,
      shopTitle: json['shop_title'] as String?,
      shopToken: json['shop_token'] as String?,
      telegramUserId: json['telegram_user_id'] as int,
      customerName: json['customer_name'] as String,
      customerPhone: json['customer_phone'] as String?,
      items: (json['items'] as List<dynamic>)
          .map((item) => CartItem.fromJson(item as Map<String, dynamic>))
          .toList(),
      totalPrice: json['total_price'] as int,
      currency: json['currency'] as String,
      status: OrderStatus.fromString(json['status'] as String),
      paymentMethod: json['payment_method'] as String?,
      notes: json['notes'] as String?,
      createdAt: DateTime.parse(json['created_at'] as String),
      updatedAt: DateTime.parse(
        json['updated_at'] as String? ?? json['created_at'] as String,
      ),
    );
  }

  Map<String, dynamic> toJson() {
    return {
      'id': id,
      'order_id': orderId,
      'shop_id': shopId,
      if (shopTitle != null) 'shop_title': shopTitle,
      if (shopToken != null) 'shop_token': shopToken,
      'telegram_user_id': telegramUserId,
      'customer_name': customerName,
      if (customerPhone != null) 'customer_phone': customerPhone,
      'items': items.map((item) => item.toJson()).toList(),
      'total_price': totalPrice,
      'currency': currency,
      'status': status.name,
      if (paymentMethod != null) 'payment_method': paymentMethod,
      if (notes != null) 'notes': notes,
      'created_at': createdAt.toIso8601String(),
      'updated_at': updatedAt.toIso8601String(),
    };
  }
}
```

---

## API клиент

### API Service

`lib/services/api_service.dart`:

```dart
import 'dart:async';
import 'dart:convert';
import 'package:http/http.dart' as http;
import '../config/api_config.dart';
import '../models/shop.dart';
import '../models/product.dart';
import '../models/cart_item.dart';
import '../models/order.dart';

class ApiException implements Exception {
  final String message;
  final int? statusCode;
  final String? code;

  ApiException(this.message, {this.statusCode, this.code});

  @override
  String toString() => 'ApiException: $message (code: $code, status: $statusCode)';
}

class ApiService {
  final String baseUrl;
  final http.Client _client;

  ApiService({
    String? baseUrl,
    http.Client? client,
  })  : baseUrl = baseUrl ?? ApiConfig.baseUrl,
        _client = client ?? http.Client();

  // ============================================================================
  // SHOP ENDPOINTS
  // ============================================================================

  /// Получить магазин по токену
  Future<Shop> getShop(String shopToken) async {
    final response = await _getWithRetry('/shops/token/$shopToken');
    return Shop.fromJson(response);
  }

  /// Создать магазин
  Future<Shop> createShop({
    required int ownerTgId,
    required String title,
    String? description,
    String currency = 'RUB',
    String? shopToken,
  }) async {
    final response = await _postWithRetry('/shops', {
      'owner_tg_id': ownerTgId,
      'title': title,
      if (description != null) 'description': description,
      'currency': currency,
      if (shopToken != null) 'shop_token': shopToken,
    });
    return Shop.fromJson(response['shop']);
  }

  // ============================================================================
  // PRODUCT ENDPOINTS
  // ============================================================================

  /// Получить все товары магазина
  Future<List<Product>> getProducts(int shopId) async {
    final response = await _getWithRetry('/products/$shopId');
    return (response as List)
        .map((json) => Product.fromJson(json as Map<String, dynamic>))
        .toList();
  }

  /// Получить детали товара
  Future<Product> getProductDetail(String productId) async {
    final response = await _getWithRetry('/products/detail/$productId');
    return Product.fromJson(response);
  }

  /// Создать товар (admin)
  Future<Product> createProduct({
    required int ownerTgId,
    required int shopId,
    required String title,
    required int price,
    String currency = 'RUB',
    String? imageUrl,
    String? description,
    int stockCount = 0,
    String? category,
  }) async {
    final response = await _postWithRetry(
      '/admin/products',
      {
        'shop_id': shopId,
        'title': title,
        'price': price,
        'currency': currency,
        if (imageUrl != null) 'image_url': imageUrl,
        if (description != null) 'description': description,
        'stock_count': stockCount,
        if (category != null) 'category': category,
      },
      headers: {'X-Owner-TG-ID': ownerTgId.toString()},
    );
    return Product.fromJson(response['product']);
  }

  // ============================================================================
  // CART ENDPOINTS
  // ============================================================================

  /// Сохранить корзину
  Future<void> saveCart({
    required int userId,
    required int shopId,
    required List<CartItem> items,
  }) async {
    await _postWithRetry('/cart', {
      'telegram_user_id': userId,
      'shop_id': shopId,
      'items': items.map((item) => item.toJson()).toList(),
    });
  }

  /// Получить корзину
  Future<List<CartItem>> getCart({
    required int userId,
    required int shopId,
  }) async {
    final response = await _getWithRetry(
      '/cart/$userId?shop_id=$shopId',
    );
    return (response['items'] as List)
        .map((json) => CartItem.fromJson(json as Map<String, dynamic>))
        .toList();
  }

  /// Очистить корзину
  Future<void> clearCart({
    required int userId,
    required int shopId,
  }) async {
    await _deleteWithRetry('/cart/$userId?shop_id=$shopId');
  }

  // ============================================================================
  // ORDER ENDPOINTS
  // ============================================================================

  /// Создать заказ
  Future<Order> createOrder({
    required int shopId,
    required int telegramUserId,
    required String customerName,
    String? customerPhone,
    required List<CartItem> items,
    required int totalPrice,
    required String currency,
    String? paymentMethod,
    String? notes,
  }) async {
    final response = await _postWithRetry('/orders', {
      'shop_id': shopId,
      'telegram_user_id': telegramUserId,
      'customer_name': customerName,
      if (customerPhone != null) 'customer_phone': customerPhone,
      'items': items.map((item) => item.toJson()).toList(),
      'total_price': totalPrice,
      'currency': currency,
      if (paymentMethod != null) 'payment_method': paymentMethod,
      if (notes != null) 'notes': notes,
    });
    return Order.fromJson(response['order']);
  }

  /// Получить все заказы пользователя (из всех магазинов)
  Future<List<Order>> getUserOrders(int userId) async {
    final response = await _getWithRetry('/user/orders/$userId');
    return (response as List)
        .map((json) => Order.fromJson(json as Map<String, dynamic>))
        .toList();
  }

  // ============================================================================
  // PRIVATE METHODS
  // ============================================================================

  Future<dynamic> _getWithRetry(
    String endpoint, {
    Map<String, String>? headers,
  }) async {
    return _retryRequest(
      () => _get(endpoint, headers: headers),
    );
  }

  Future<dynamic> _postWithRetry(
    String endpoint,
    Map<String, dynamic> body, {
    Map<String, String>? headers,
  }) async {
    return _retryRequest(
      () => _post(endpoint, body, headers: headers),
    );
  }

  Future<dynamic> _deleteWithRetry(
    String endpoint, {
    Map<String, String>? headers,
  }) async {
    return _retryRequest(
      () => _delete(endpoint, headers: headers),
    );
  }

  Future<dynamic> _retryRequest(
    Future<dynamic> Function() request,
  ) async {
    int attempts = 0;
    while (attempts < ApiConfig.maxRetries) {
      try {
        return await request();
      } on ApiException catch (e) {
        // Не повторять для клиентских ошибок (4xx)
        if (e.statusCode != null && e.statusCode! >= 400 && e.statusCode! < 500) {
          rethrow;
        }
        attempts++;
        if (attempts >= ApiConfig.maxRetries) rethrow;
        await Future.delayed(ApiConfig.retryDelay * attempts);
      }
    }
  }

  Future<dynamic> _get(
    String endpoint, {
    Map<String, String>? headers,
  }) async {
    final url = Uri.parse('$baseUrl$endpoint');
    final response = await _client
        .get(url, headers: _buildHeaders(headers))
        .timeout(ApiConfig.connectionTimeout);

    return _handleResponse(response);
  }

  Future<dynamic> _post(
    String endpoint,
    Map<String, dynamic> body, {
    Map<String, String>? headers,
  }) async {
    final url = Uri.parse('$baseUrl$endpoint');
    final response = await _client
        .post(
          url,
          headers: _buildHeaders(headers),
          body: json.encode(body),
        )
        .timeout(ApiConfig.connectionTimeout);

    return _handleResponse(response);
  }

  Future<dynamic> _delete(
    String endpoint, {
    Map<String, String>? headers,
  }) async {
    final url = Uri.parse('$baseUrl$endpoint');
    final response = await _client
        .delete(url, headers: _buildHeaders(headers))
        .timeout(ApiConfig.connectionTimeout);

    return _handleResponse(response);
  }

  Map<String, String> _buildHeaders(Map<String, String>? customHeaders) {
    return {
      'Content-Type': 'application/json',
      'Accept': 'application/json',
      ...?customHeaders,
    };
  }

  dynamic _handleResponse(http.Response response) {
    if (response.statusCode >= 200 && response.statusCode < 300) {
      if (response.body.isEmpty) return null;
      return json.decode(response.body);
    }

    final errorBody = json.decode(response.body);
    throw ApiException(
      errorBody['error'] ?? 'Unknown error',
      statusCode: response.statusCode,
      code: errorBody['code'],
    );
  }

  void dispose() {
    _client.close();
  }
}
```

---

## State Management

### Cart Provider

`lib/providers/cart_provider.dart`:

```dart
import 'package:flutter/foundation.dart';
import '../models/cart_item.dart';
import '../models/product.dart';
import '../services/api_service.dart';

class CartProvider with ChangeNotifier {
  final ApiService _apiService;
  final int userId;
  int? _shopId;

  List<CartItem> _items = [];
  bool _isLoading = false;
  bool _isSaving = false;
  String? _error;

  CartProvider({
    required ApiService apiService,
    required this.userId,
  }) : _apiService = apiService;

  List<CartItem> get items => _items;
  bool get isLoading => _isLoading;
  bool get isSaving => _isSaving;
  String? get error => _error;
  int get itemCount => _items.fold(0, (sum, item) => sum + item.quantity);
  int get totalPrice => _items.fold(0, (sum, item) => sum + item.totalPrice);

  /// Загрузить корзину с сервера
  Future<void> loadCart(int shopId) async {
    _shopId = shopId;
    _isLoading = true;
    _error = null;
    notifyListeners();

    try {
      _items = await _apiService.getCart(userId: userId, shopId: shopId);
      _error = null;
    } catch (e) {
      _error = e.toString();
      _items = [];
    } finally {
      _isLoading = false;
      notifyListeners();
    }
  }

  /// Добавить товар в корзину
  Future<void> addProduct(Product product, {int quantity = 1}) async {
    if (_shopId == null) return;

    final existingIndex = _items.indexWhere(
      (item) => item.productId == product.productId,
    );

    if (existingIndex >= 0) {
      _items[existingIndex].quantity += quantity;
    } else {
      _items.add(CartItem.fromProduct(product, quantity: quantity));
    }

    notifyListeners();
    await _saveToServer();
  }

  /// Удалить товар из корзины
  Future<void> removeProduct(String productId) async {
    _items.removeWhere((item) => item.productId == productId);
    notifyListeners();
    await _saveToServer();
  }

  /// Обновить количество товара
  Future<void> updateQuantity(String productId, int quantity) async {
    if (quantity <= 0) {
      await removeProduct(productId);
      return;
    }

    final index = _items.indexWhere((item) => item.productId == productId);
    if (index >= 0) {
      _items[index].quantity = quantity;
      notifyListeners();
      await _saveToServer();
    }
  }

  /// Очистить корзину
  Future<void> clear() async {
    if (_shopId == null) return;

    _items.clear();
    notifyListeners();

    try {
      await _apiService.clearCart(userId: userId, shopId: _shopId!);
    } catch (e) {
      _error = e.toString();
    }
  }

  /// Сохранить на сервер
  Future<void> _saveToServer() async {
    if (_shopId == null) return;

    _isSaving = true;
    notifyListeners();

    try {
      await _apiService.saveCart(
        userId: userId,
        shopId: _shopId!,
        items: _items,
      );
      _error = null;
    } catch (e) {
      _error = e.toString();
    } finally {
      _isSaving = false;
      notifyListeners();
    }
  }
}
```

---

## UI компоненты

### Shop Screen

`lib/screens/shop_screen.dart`:

```dart
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import '../models/product.dart';
import '../providers/cart_provider.dart';
import '../services/api_service.dart';

class ShopScreen extends StatefulWidget {
  final String shopToken;

  const ShopScreen({Key? key, required this.shopToken}) : super(key: key);

  @override
  State<ShopScreen> createState() => _ShopScreenState();
}

class _ShopScreenState extends State<ShopScreen> {
  late Future<List<Product>> _productsFuture;
  final ApiService _apiService = ApiService();

  @override
  void initState() {
    super.initState();
    _loadShopAndProducts();
  }

  Future<void> _loadShopAndProducts() async {
    try {
      final shop = await _apiService.getShop(widget.shopToken);
      setState(() {
        _productsFuture = _apiService.getProducts(shop.id);
      });
      
      // Загрузить корзину
      if (mounted) {
        await context.read<CartProvider>().loadCart(shop.id);
      }
    } catch (e) {
      // Обработка ошибок
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text('Ошибка загрузки: $e')),
        );
      }
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('ShopKit'),
        actions: [
          Consumer<CartProvider>(
            builder: (context, cart, child) {
              return Stack(
                children: [
                  IconButton(
                    icon: const Icon(Icons.shopping_cart),
                    onPressed: () {
                      Navigator.pushNamed(context, '/cart');
                    },
                  ),
                  if (cart.itemCount > 0)
                    Positioned(
                      right: 8,
                      top: 8,
                      child: Container(
                        padding: const EdgeInsets.all(2),
                        decoration: BoxDecoration(
                          color: Colors.red,
                          borderRadius: BorderRadius.circular(10),
                        ),
                        constraints: const BoxConstraints(
                          minWidth: 16,
                          minHeight: 16,
                        ),
                        child: Text(
                          '${cart.itemCount}',
                          style: const TextStyle(
                            color: Colors.white,
                            fontSize: 10,
                          ),
                          textAlign: TextAlign.center,
                        ),
                      ),
                    ),
                ],
              );
            },
          ),
        ],
      ),
      body: FutureBuilder<List<Product>>(
        future: _productsFuture,
        builder: (context, snapshot) {
          if (snapshot.connectionState == ConnectionState.waiting) {
            return const Center(child: CircularProgressIndicator());
          }

          if (snapshot.hasError) {
            return Center(
              child: Column(
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  const Icon(Icons.error, size: 48, color: Colors.red),
                  const SizedBox(height: 16),
                  Text('Ошибка: ${snapshot.error}'),
                  const SizedBox(height: 16),
                  ElevatedButton(
                    onPressed: _loadShopAndProducts,
                    child: const Text('Повторить'),
                  ),
                ],
              ),
            );
          }

          final products = snapshot.data!;

          if (products.isEmpty) {
            return const Center(
              child: Text('Товары не найдены'),
            );
          }

          return GridView.builder(
            padding: const EdgeInsets.all(8),
            gridDelegate: const SliverGridDelegateWithFixedCrossAxisCount(
              crossAxisCount: 2,
              childAspectRatio: 0.7,
              crossAxisSpacing: 8,
              mainAxisSpacing: 8,
            ),
            itemCount: products.length,
            itemBuilder: (context, index) {
              return ProductCard(product: products[index]);
            },
          );
        },
      ),
    );
  }

  @override
  void dispose() {
    _apiService.dispose();
    super.dispose();
  }
}

class ProductCard extends StatelessWidget {
  final Product product;

  const ProductCard({Key? key, required this.product}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Card(
      clipBehavior: Clip.antiAlias,
      child: InkWell(
        onTap: () {
          Navigator.pushNamed(
            context,
            '/product',
            arguments: product,
          );
        },
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Expanded(
              child: product.imageUrl != null
                  ? Image.network(
                      product.imageUrl!,
                      fit: BoxFit.cover,
                      width: double.infinity,
                      errorBuilder: (context, error, stackTrace) {
                        return Container(
                          color: Colors.grey[200],
                          child: const Icon(Icons.image, size: 48),
                        );
                      },
                    )
                  : Container(
                      color: Colors.grey[200],
                      child: const Icon(Icons.image, size: 48),
                    ),
            ),
            Padding(
              padding: const EdgeInsets.all(8.0),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Text(
                    product.title,
                    style: const TextStyle(
                      fontWeight: FontWeight.bold,
                      fontSize: 14,
                    ),
                    maxLines: 2,
                    overflow: TextOverflow.ellipsis,
                  ),
                  const SizedBox(height: 4),
                  Text(
                    product.formattedPrice,
                    style: const TextStyle(
                      color: Colors.green,
                      fontWeight: FontWeight.bold,
                    ),
                  ),
                  const SizedBox(height: 8),
                  SizedBox(
                    width: double.infinity,
                    child: ElevatedButton(
                      onPressed: product.isAvailable
                          ? () {
                              context.read<CartProvider>().addProduct(product);
                              ScaffoldMessenger.of(context).showSnackBar(
                                SnackBar(
                                  content: Text('${product.title} добавлен'),
                                  duration: const Duration(seconds: 1),
                                ),
                              );
                            }
                          : null,
                      child: const Text('В корзину'),
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

### Cart Screen

`lib/screens/cart_screen.dart`:

```dart
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import '../providers/cart_provider.dart';

class CartScreen extends StatelessWidget {
  const CartScreen({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Корзина'),
      ),
      body: Consumer<CartProvider>(
        builder: (context, cart, child) {
          if (cart.isLoading) {
            return const Center(child: CircularProgressIndicator());
          }

          if (cart.items.isEmpty) {
            return Center(
              child: Column(
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  const Icon(Icons.shopping_cart_outlined, size: 64),
                  const SizedBox(height: 16),
                  const Text('Корзина пуста'),
                  const SizedBox(height: 16),
                  ElevatedButton(
                    onPressed: () {
                      Navigator.pop(context);
                    },
                    child: const Text('Продолжить покупки'),
                  ),
                ],
              ),
            );
          }

          return Column(
            children: [
              Expanded(
                child: ListView.builder(
                  itemCount: cart.items.length,
                  itemBuilder: (context, index) {
                    final item = cart.items[index];
                    return Card(
                      margin: const EdgeInsets.symmetric(
                        horizontal: 8,
                        vertical: 4,
                      ),
                      child: Padding(
                        padding: const EdgeInsets.all(8.0),
                        child: Row(
                          children: [
                            Expanded(
                              child: Column(
                                crossAxisAlignment: CrossAxisAlignment.start,
                                children: [
                                  Text(
                                    item.title,
                                    style: const TextStyle(
                                      fontWeight: FontWeight.bold,
                                    ),
                                  ),
                                  const SizedBox(height: 4),
                                  Text(
                                    '${item.price / 100} руб × ${item.quantity}',
                                    style: TextStyle(
                                      color: Colors.grey[600],
                                    ),
                                  ),
                                ],
                              ),
                            ),
                            Row(
                              children: [
                                IconButton(
                                  icon: const Icon(Icons.remove),
                                  onPressed: () {
                                    cart.updateQuantity(
                                      item.productId,
                                      item.quantity - 1,
                                    );
                                  },
                                ),
                                Text('${item.quantity}'),
                                IconButton(
                                  icon: const Icon(Icons.add),
                                  onPressed: () {
                                    cart.updateQuantity(
                                      item.productId,
                                      item.quantity + 1,
                                    );
                                  },
                                ),
                              ],
                            ),
                            Text(
                              '${item.totalPrice / 100} руб',
                              style: const TextStyle(
                                fontWeight: FontWeight.bold,
                              ),
                            ),
                            IconButton(
                              icon: const Icon(Icons.delete, color: Colors.red),
                              onPressed: () {
                                cart.removeProduct(item.productId);
                              },
                            ),
                          ],
                        ),
                      ),
                    );
                  },
                ),
              ),
              Container(
                padding: const EdgeInsets.all(16),
                decoration: BoxDecoration(
                  color: Colors.white,
                  boxShadow: [
                    BoxShadow(
                      color: Colors.black.withOpacity(0.1),
                      blurRadius: 4,
                      offset: const Offset(0, -2),
                    ),
                  ],
                ),
                child: SafeArea(
                  child: Column(
                    children: [
                      Row(
                        mainAxisAlignment: MainAxisAlignment.spaceBetween,
                        children: [
                          const Text(
                            'Итого:',
                            style: TextStyle(
                              fontSize: 18,
                              fontWeight: FontWeight.bold,
                            ),
                          ),
                          Text(
                            '${cart.totalPrice / 100} руб',
                            style: const TextStyle(
                              fontSize: 18,
                              fontWeight: FontWeight.bold,
                              color: Colors.green,
                            ),
                          ),
                        ],
                      ),
                      const SizedBox(height: 16),
                      SizedBox(
                        width: double.infinity,
                        child: ElevatedButton(
                          onPressed: cart.isSaving
                              ? null
                              : () {
                                  Navigator.pushNamed(context, '/checkout');
                                },
                          style: ElevatedButton.styleFrom(
                            padding: const EdgeInsets.symmetric(vertical: 16),
                          ),
                          child: cart.isSaving
                              ? const SizedBox(
                                  height: 20,
                                  width: 20,
                                  child: CircularProgressIndicator(
                                    strokeWidth: 2,
                                  ),
                                )
                              : const Text(
                                  'Оформить заказ',
                                  style: TextStyle(fontSize: 16),
                                ),
                        ),
                      ),
                    ],
                  ),
                ),
              ),
            ],
          );
        },
      ),
    );
  }
}
```

---

## Навигация

### Main App

`lib/main.dart`:

```dart
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'screens/shop_screen.dart';
import 'screens/cart_screen.dart';
import 'screens/checkout_screen.dart';
import 'screens/orders_screen.dart';
import 'providers/cart_provider.dart';
import 'services/api_service.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    // TODO: Получить userId из аутентификации
    const int userId = 123456789;

    return MultiProvider(
      providers: [
        Provider(create: (_) => ApiService()),
        ChangeNotifierProvider(
          create: (context) => CartProvider(
            apiService: context.read<ApiService>(),
            userId: userId,
          ),
        ),
      ],
      child: MaterialApp(
        title: 'ShopKit',
        theme: ThemeData(
          colorScheme: ColorScheme.fromSeed(seedColor: Colors.blue),
          useMaterial3: true,
        ),
        initialRoute: '/',
        routes: {
          '/': (context) => const ShopScreen(shopToken: 'demo'),
          '/cart': (context) => const CartScreen(),
          '/checkout': (context) => const CheckoutScreen(),
          '/orders': (context) => const OrdersScreen(),
        },
      ),
    );
  }
}
```

---

## Платформо-специфичные фичи

### Android

#### AndroidManifest.xml

```xml
<manifest xmlns:android="http://schemas.android.com/apk/res/android">
    <!-- Интернет разрешение -->
    <uses-permission android:name="android.permission.INTERNET" />
    
    <!-- Опционально: доступ к камере для сканирования QR -->
    <uses-permission android:name="android.permission.CAMERA" />
    
    <application
        android:label="ShopKit"
        android:icon="@mipmap/ic_launcher">
        
        <!-- Deep linking -->
        <intent-filter>
            <action android:name="android.intent.action.VIEW" />
            <category android:name="android.intent.category.DEFAULT" />
            <category android:name="android.intent.category.BROWSABLE" />
            <data
                android:scheme="shopkit"
                android:host="shop" />
        </intent-filter>
    </application>
</manifest>
```

#### build.gradle (app level)

```gradle
android {
    namespace "com.example.shopkit_app"
    compileSdk 34

    defaultConfig {
        applicationId "com.example.shopkit_app"
        minSdk 21
        targetSdk 34
        versionCode 1
        versionName "1.0.0"
    }

    buildTypes {
        release {
            signingConfig signingConfigs.release
            minifyEnabled true
            shrinkResources true
        }
    }
}
```

### iOS

#### Info.plist

```xml
<dict>
    <!-- Описание использования камеры -->
    <key>NSCameraUsageDescription</key>
    <string>Используется для сканирования QR кодов</string>
    
    <!-- Deep linking -->
    <key>CFBundleURLTypes</key>
    <array>
        <dict>
            <key>CFBundleURLSchemes</key>
            <array>
                <string>shopkit</string>
            </array>
        </dict>
    </array>
    
    <!-- HTTP разрешения -->
    <key>NSAppTransportSecurity</key>
    <dict>
        <key>NSAllowsArbitraryLoads</key>
        <false/>
    </dict>
</dict>
```

---

## Тестирование

### Unit Tests

`test/models/cart_item_test.dart`:

```dart
import 'package:flutter_test/flutter_test.dart';
import 'package:shopkit_app/models/cart_item.dart';

void main() {
  group('CartItem', () {
    test('calculates total price correctly', () {
      final item = CartItem(
        productId: 'test-id',
        title: 'Test Product',
        quantity: 3,
        price: 250,
      );

      expect(item.totalPrice, 750);
    });

    test('fromJson creates valid CartItem', () {
      final json = {
        'product_id': 'test-id',
        'title': 'Test Product',
        'quantity': 2,
        'price': 100,
      };

      final item = CartItem.fromJson(json);

      expect(item.productId, 'test-id');
      expect(item.title, 'Test Product');
      expect(item.quantity, 2);
      expect(item.price, 100);
    });

    test('toJson returns correct map', () {
      final item = CartItem(
        productId: 'test-id',
        title: 'Test Product',
        quantity: 1,
        price: 500,
      );

      final json = item.toJson();

      expect(json['product_id'], 'test-id');
      expect(json['title'], 'Test Product');
      expect(json['quantity'], 1);
      expect(json['price'], 500);
    });
  });
}
```

### Widget Tests

`test/widgets/product_card_test.dart`:

```dart
import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:shopkit_app/models/product.dart';
import 'package:shopkit_app/screens/shop_screen.dart';

void main() {
  testWidgets('ProductCard displays product info', (WidgetTester tester) async {
    final product = Product(
      id: 1,
      productId: 'test-id',
      shopId: 1,
      title: 'Test Product',
      price: 250,
      currency: 'RUB',
      stockCount: 10,
      isActive: true,
      createdAt: DateTime.now(),
      updatedAt: DateTime.now(),
    );

    await tester.pumpWidget(
      MaterialApp(
        home: Scaffold(
          body: ProductCard(product: product),
        ),
      ),
    );

    expect(find.text('Test Product'), findsOneWidget);
    expect(find.text('2.50 RUB'), findsOneWidget);
    expect(find.text('В корзину'), findsOneWidget);
  });
}
```

### Integration Tests

`integration_test/app_test.dart`:

```dart
import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:shopkit_app/main.dart' as app;

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  group('end-to-end test', () {
    testWidgets('complete shopping flow', (tester) async {
      app.main();
      await tester.pumpAndSettle();

      // Найти первый товар
      final productCard = find.byType(Card).first;
      expect(productCard, findsOneWidget);

      // Добавить в корзину
      await tester.tap(find.text('В корзину').first);
      await tester.pumpAndSettle();

      // Проверить badge корзины
      expect(find.text('1'), findsOneWidget);

      // Перейти в корзину
      await tester.tap(find.byIcon(Icons.shopping_cart));
      await tester.pumpAndSettle();

      // Проверить наличие товара
      expect(find.text('Test Product'), findsOneWidget);

      // Оформить заказ
      await tester.tap(find.text('Оформить заказ'));
      await tester.pumpAndSettle();
    });
  });
}
```

---

## Деплой

### Android (Google Play Store)

#### 1. Создание keystore

```bash
keytool -genkey -v -keystore ~/shopkit-release-key.jks \
  -keyalg RSA -keysize 2048 -validity 10000 \
  -alias shopkit
```

#### 2. Конфигурация подписи

Создайте `android/key.properties`:

```properties
storePassword=YOUR_STORE_PASSWORD
keyPassword=YOUR_KEY_PASSWORD
keyAlias=shopkit
storeFile=/path/to/shopkit-release-key.jks
```

#### 3. Обновите `android/app/build.gradle`:

```gradle
def keystoreProperties = new Properties()
def keystorePropertiesFile = rootProject.file('key.properties')
if (keystorePropertiesFile.exists()) {
    keystoreProperties.load(new FileInputStream(keystorePropertiesFile))
}

android {
    signingConfigs {
        release {
            keyAlias keystoreProperties['keyAlias']
            keyPassword keystoreProperties['keyPassword']
            storeFile keystoreProperties['storeFile'] ? file(keystoreProperties['storeFile']) : null
            storePassword keystoreProperties['storePassword']
        }
    }
    buildTypes {
        release {
            signingConfig signingConfigs.release
        }
    }
}
```

#### 4. Сборка APK/AAB

```bash
# APK (для тестирования)
flutter build apk --release

# AAB (для Google Play)
flutter build appbundle --release
```

#### 5. Загрузка в Play Console

1. Создайте приложение на [Google Play Console](https://play.google.com/console)
2. Заполните информацию о приложении
3. Загрузите AAB файл
4. Пройдите процесс проверки
5. Опубликуйте приложение

### iOS (App Store)

#### 1. Настройка в Xcode

```bash
# Открыть проект в Xcode
open ios/Runner.xcworkspace

# В Xcode:
# - Signing & Capabilities
# - Выберите Team
# - Установите Bundle Identifier
```

#### 2. Конфигурация Info.plist

```xml
<key>CFBundleDisplayName</key>
<string>ShopKit</string>
<key>CFBundleVersion</key>
<string>1</string>
<key>CFBundleShortVersionString</key>
<string>1.0.0</string>
```

#### 3. Сборка IPA

```bash
# Сборка для iOS
flutter build ios --release

# Или с архивированием
flutter build ipa --release
```

#### 4. Загрузка в App Store Connect

1. Создайте приложение на [App Store Connect](https://appstoreconnect.apple.com)
2. Используйте Xcode или Transporter для загрузки IPA
3. Заполните метаданные приложения
4. Отправьте на проверку

---

## Best Practices

### 1. Обработка ошибок

```dart
// Глобальный обработчик ошибок
void main() {
  FlutterError.onError = (details) {
    // Логирование ошибок
    print('Flutter Error: ${details.exception}');
    // Отправка в систему мониторинга (Firebase Crashlytics, Sentry)
  };

  runApp(const MyApp());
}

// Обработка асинхронных ошибок
Future<void> loadData() async {
  try {
    final data = await apiService.getData();
    // Обработка данных
  } on ApiException catch (e) {
    // Специфичная ошибка API
    showError('API Error: ${e.message}');
  } catch (e) {
    // Общая ошибка
    showError('Unexpected error: $e');
  }
}
```

### 2. Кэширование

```dart
import 'package:shared_preferences/shared_preferences.dart';

class CacheService {
  static const String _cartKey = 'cart_cache';
  
  Future<void> saveCart(List<CartItem> items) async {
    final prefs = await SharedPreferences.getInstance();
    final json = items.map((item) => item.toJson()).toList();
    await prefs.setString(_cartKey, jsonEncode(json));
  }
  
  Future<List<CartItem>?> loadCart() async {
    final prefs = await SharedPreferences.getInstance();
    final json = prefs.getString(_cartKey);
    if (json == null) return null;
    
    final List<dynamic> decoded = jsonDecode(json);
    return decoded.map((item) => CartItem.fromJson(item)).toList();
  }
}
```

### 3. Оптимизация изображений

```dart
import 'package:cached_network_image/cached_network_image.dart';

Widget buildProductImage(String? imageUrl) {
  if (imageUrl == null) {
    return const Placeholder();
  }
  
  return CachedNetworkImage(
    imageUrl: imageUrl,
    placeholder: (context, url) => const CircularProgressIndicator(),
    errorWidget: (context, url, error) => const Icon(Icons.error),
    fit: BoxFit.cover,
  );
}
```

### 4. Pagination

```dart
class ProductListScreen extends StatefulWidget {
  @override
  State<ProductListScreen> createState() => _ProductListScreenState();
}

class _ProductListScreenState extends State<ProductListScreen> {
  final ScrollController _scrollController = ScrollController();
  final List<Product> _products = [];
  int _page = 1;
  bool _isLoading = false;
  bool _hasMore = true;

  @override
  void initState() {
    super.initState();
    _loadProducts();
    _scrollController.addListener(_onScroll);
  }

  void _onScroll() {
    if (_scrollController.position.pixels >=
        _scrollController.position.maxScrollExtent - 200) {
      _loadProducts();
    }
  }

  Future<void> _loadProducts() async {
    if (_isLoading || !_hasMore) return;
    
    setState(() => _isLoading = true);
    
    try {
      final newProducts = await apiService.getProducts(
        shopId: shopId,
        page: _page,
      );
      
      setState(() {
        _products.addAll(newProducts);
        _page++;
        _hasMore = newProducts.isNotEmpty;
      });
    } finally {
      setState(() => _isLoading = false);
    }
  }

  @override
  Widget build(BuildContext context) {
    return ListView.builder(
      controller: _scrollController,
      itemCount: _products.length + (_hasMore ? 1 : 0),
      itemBuilder: (context, index) {
        if (index == _products.length) {
          return const Center(child: CircularProgressIndicator());
        }
        return ProductCard(product: _products[index]);
      },
    );
  }

  @override
  void dispose() {
    _scrollController.dispose();
    super.dispose();
  }
}
```

### 5. Локализация

```dart
// l10n/app_en.arb
{
  "appTitle": "ShopKit",
  "addToCart": "Add to Cart",
  "checkout": "Checkout",
  "total": "Total"
}

// l10n/app_ru.arb
{
  "appTitle": "ШопКит",
  "addToCart": "В корзину",
  "checkout": "Оформить заказ",
  "total": "Итого"
}

// pubspec.yaml
flutter:
  generate: true

// Использование
Text(AppLocalizations.of(context)!.addToCart)
```

---

## Troubleshooting

### Проблема: Ошибка сети на Android

**Решение:**
```xml
<!-- AndroidManifest.xml -->
<uses-permission android:name="android.permission.INTERNET" />
<application android:usesCleartextTraffic="true">
```

### Проблема: iOS build fails

**Решение:**
```bash
cd ios
pod deintegrate
pod install
cd ..
flutter clean
flutter build ios
```

### Проблема: Hot reload не работает

**Решение:**
```bash
flutter clean
flutter pub get
flutter run
```

### Проблема: Медленная загрузка изображений

**Решение:** Используйте `cached_network_image`:
```dart
CachedNetworkImage(
  imageUrl: imageUrl,
  memCacheHeight: 200,
  memCacheWidth: 200,
)
```

---

## Дополнительные ресурсы

- **API Reference:** [API_ENDPOINTS.md](./API_ENDPOINTS.md)
- **Flutter Docs:** https://docs.flutter.dev
- **Provider Docs:** https://pub.dev/packages/provider
- **HTTP Package:** https://pub.dev/packages/http

---

## Примеры запуска

```bash
# Development
flutter run --debug

# Profile (для тестирования производительности)
flutter run --profile

# Release
flutter run --release

# На конкретном устройстве
flutter run -d <device_id>

# Список устройств
flutter devices
```

---

**Готово!** Теперь у вас есть полное руководство по разработке Flutter приложения для Android и iOS с интеграцией ShopKit backend. 🚀
