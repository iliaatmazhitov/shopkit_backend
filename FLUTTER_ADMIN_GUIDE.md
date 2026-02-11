# 📱 Flutter Admin App - Полное руководство для владельцев магазинов

> **Production API:** `https://shopkit-backend.onrender.com/api`  
> **Status:** ✅ Production-ready

## 🎯 О чём этот гайд?

Это **полное руководство** для Flutter-разработчика, который создаёт мобильное приложение (Android/iOS) для **владельцев магазинов**.

### Важно понять архитектуру системы:

```
┌─────────────────────────┐
│  Покупатели (клиенты)   │
│                         │
│  Telegram Mini App      │ ← Веб-приложение в Telegram
│  (React/TypeScript)     │   Просмотр товаров, корзина,
│                         │   оформление заказов
└────────────┬────────────┘
             │
             │ API запросы
             │
┌────────────▼────────────┐
│   ShopKit Backend       │
│   (C++ Drogon)          │ ← Сервер на Render.com
│   PostgreSQL            │
└────────────┬────────────┘
             │
             │ API запросы
             │
┌────────────▼────────────┐
│  Владельцы магазинов    │
│                         │
│  Flutter Mobile App     │ ← Это приложение!
│  (Android + iOS)        │   Управление магазином,
│                         │   товарами, заказами
└─────────────────────────┘
```

### Что делает это приложение (Admin App)?

**Для владельцев магазинов:**
- ✅ Создание и настройка магазина
- ✅ Добавление, редактирование, удаление товаров
- ✅ Загрузка фотографий товаров
- ✅ Управление остатками (stock)
- ✅ Просмотр заказов клиентов
- ✅ Изменение статусов заказов (pending → completed/cancelled)
- ✅ Просмотр статистики

### Чего НЕТ в Admin приложении?

**Не нужно (это делают покупатели в TMA):**
- ❌ Корзина покупок
- ❌ Оформление заказа
- ❌ Добавление товаров в корзину
- ❌ Оплата

---

## 📋 Содержание

1. [Quick Start](#quick-start)
2. [Структура проекта](#структура-проекта)
3. [Модели данных](#модели-данных)
4. [Аутентификация](#аутентификация)
5. [API Service](#api-service)
6. [State Management](#state-management)
7. [UI Screens](#ui-screens)
8. [Навигация](#навигация)
9. [Загрузка изображений](#загрузка-изображений)
10. [Тестирование](#тестирование)
11. [Деплой](#деплой)
12. [Troubleshooting](#troubleshooting)

---

## Quick Start

### Требования

- Flutter SDK 3.16.0+
- Dart 3.2.0+
- Android Studio / Xcode
- Telegram аккаунт (для получения owner_tg_id)

### Создание проекта

```bash
# Создать Flutter проект
flutter create shopkit_admin
cd shopkit_admin

# Добавить зависимости
flutter pub add http provider shared_preferences image_picker intl
flutter pub add --dev flutter_lints

# Запустить
flutter run
```

### pubspec.yaml

```yaml
name: shopkit_admin
description: Admin app for ShopKit store owners
version: 1.0.0+1

environment:
  sdk: ">=3.2.0 <4.0.0"

dependencies:
  flutter:
    sdk: flutter
  
  # HTTP клиент
  http: ^1.1.0
  
  # State Management
  provider: ^6.1.1
  
  # Локальное хранилище (для сохранения auth данных)
  shared_preferences: ^2.2.2
  
  # Выбор и загрузка изображений
  image_picker: ^1.0.5
  
  # Форматирование (даты, цены)
  intl: ^0.18.1

dev_dependencies:
  flutter_test:
    sdk: flutter
  flutter_lints: ^3.0.0
```

---

## Структура проекта

Рекомендуемая структура для admin приложения:

```
lib/
├── main.dart                    # Entry point
├── config/
│   └── api_config.dart         # API URLs и константы
├── models/
│   ├── shop.dart               # Модель магазина
│   ├── product.dart            # Модель товара
│   ├── order.dart              # Модель заказа
│   └── order_item.dart         # Модель товара в заказе
├── services/
│   ├── auth_service.dart       # Аутентификация (хранение owner_tg_id)
│   └── api_service.dart        # HTTP запросы к API
├── providers/
│   ├── auth_provider.dart      # State для аутентификации
│   ├── shop_provider.dart      # State для магазина
│   ├── products_provider.dart  # State для товаров
│   └── orders_provider.dart    # State для заказов
├── screens/
│   ├── auth/
│   │   └── auth_screen.dart              # Вход (ввод Telegram ID)
│   ├── shop/
│   │   ├── shop_setup_screen.dart        # Создание магазина
│   │   └── shop_settings_screen.dart     # Настройки магазина
│   ├── products/
│   │   ├── products_list_screen.dart     # Список товаров
│   │   ├── product_form_screen.dart      # Добавление/редактирование товара
│   │   └── product_detail_screen.dart    # Детали товара
│   ├── orders/
│   │   ├── orders_list_screen.dart       # Список заказов
│   │   └── order_detail_screen.dart      # Детали заказа
│   └── dashboard/
│       └── dashboard_screen.dart         # Главный экран со статистикой
└── widgets/
    ├── product_card.dart         # Карточка товара
    ├── order_card.dart           # Карточка заказа
    └── stat_card.dart            # Карточка статистики
```

---

## Модели данных

### config/api_config.dart

```dart
class ApiConfig {
  // Production API
  static const String baseUrl = 'https://shopkit-backend.onrender.com/api';
  
  // Endpoints
  static const String shops = '/shops';
  static const String shopsToken = '/shops/token';
  static const String products = '/products';
  static const String adminProducts = '/admin/products';
  static const String orders = '/orders';
  static const String userOrders = '/user/orders';
  
  // Timeouts
  static const Duration requestTimeout = Duration(seconds: 30);
  static const Duration connectTimeout = Duration(seconds: 10);
}
```

### models/shop.dart

```dart
class Shop {
  final int id;
  final String shopToken;
  final int ownerTgId;
  final String title;
  final String? description;
  final String currency;
  final DateTime createdAt;
  final DateTime updatedAt;
  final int? productsCount;

  Shop({
    required this.id,
    required this.shopToken,
    required this.ownerTgId,
    required this.title,
    this.description,
    required this.currency,
    required this.createdAt,
    required this.updatedAt,
    this.productsCount,
  });

  factory Shop.fromJson(Map<String, dynamic> json) {
    return Shop(
      id: json['id'],
      shopToken: json['shop_token'],
      ownerTgId: json['owner_tg_id'],
      title: json['title'],
      description: json['description'],
      currency: json['currency'] ?? 'RUB',
      createdAt: DateTime.parse(json['created_at']),
      updatedAt: DateTime.parse(json['updated_at']),
      productsCount: json['products_count'],
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
    };
  }
}
```

### models/product.dart

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

  // Удобные геттеры
  String get formattedPrice {
    final rubles = price / 100;
    return '${rubles.toStringAsFixed(2)} $currency';
  }

  bool get isInStock => stockCount > 0;
  
  bool get isAvailable => isActive && isInStock;

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
      createdAt: DateTime.parse(json['created_at']),
      updatedAt: DateTime.parse(json['updated_at']),
    );
  }

  Map<String, dynamic> toJson() {
    return {
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
    };
  }

  // Копия с изменениями (для редактирования)
  Product copyWith({
    int? id,
    String? productId,
    int? shopId,
    String? title,
    int? price,
    String? currency,
    String? imageUrl,
    String? description,
    int? stockCount,
    bool? isActive,
    String? category,
    List<String>? tags,
    DateTime? createdAt,
    DateTime? updatedAt,
  }) {
    return Product(
      id: id ?? this.id,
      productId: productId ?? this.productId,
      shopId: shopId ?? this.shopId,
      title: title ?? this.title,
      price: price ?? this.price,
      currency: currency ?? this.currency,
      imageUrl: imageUrl ?? this.imageUrl,
      description: description ?? this.description,
      stockCount: stockCount ?? this.stockCount,
      isActive: isActive ?? this.isActive,
      category: category ?? this.category,
      tags: tags ?? this.tags,
      createdAt: createdAt ?? this.createdAt,
      updatedAt: updatedAt ?? this.updatedAt,
    );
  }
}
```

### models/order_item.dart

```dart
class OrderItem {
  final String productId;
  final String title;
  final int quantity;
  final int price; // цена за единицу в копейках

  OrderItem({
    required this.productId,
    required this.title,
    required this.quantity,
    required this.price,
  });

  int get totalPrice => quantity * price;

  factory OrderItem.fromJson(Map<String, dynamic> json) {
    return OrderItem(
      productId: json['product_id'],
      title: json['title'],
      quantity: json['quantity'],
      price: json['price'],
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
}
```

### models/order.dart

```dart
import 'order_item.dart';

class Order {
  final int id;
  final String orderId; // UUID
  final int shopId;
  final String? shopTitle;
  final String? shopToken;
  final int telegramUserId;
  final String customerName;
  final String? customerPhone;
  final List<OrderItem> items;
  final int totalPrice;
  final String currency;
  final String status; // 'pending', 'completed', 'cancelled'
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

  String get formattedTotalPrice {
    final rubles = totalPrice / 100;
    return '${rubles.toStringAsFixed(2)} $currency';
  }

  int get itemsCount {
    return items.fold(0, (sum, item) => sum + item.quantity);
  }

  bool get isPending => status == 'pending';
  bool get isCompleted => status == 'completed';
  bool get isCancelled => status == 'cancelled';

  factory Order.fromJson(Map<String, dynamic> json) {
    return Order(
      id: json['id'],
      orderId: json['order_id'],
      shopId: json['shop_id'],
      shopTitle: json['shop_title'],
      shopToken: json['shop_token'],
      telegramUserId: json['telegram_user_id'],
      customerName: json['customer_name'] ?? 'Unknown',
      customerPhone: json['customer_phone'],
      items: (json['items'] as List)
          .map((item) => OrderItem.fromJson(item))
          .toList(),
      totalPrice: json['total_price'],
      currency: json['currency'] ?? 'RUB',
      status: json['status'] ?? 'pending',
      paymentMethod: json['payment_method'],
      notes: json['notes'],
      createdAt: DateTime.parse(json['created_at']),
      updatedAt: DateTime.parse(json['updated_at']),
    );
  }

  Map<String, dynamic> toJson() {
    return {
      'id': id,
      'order_id': orderId,
      'shop_id': shopId,
      'telegram_user_id': telegramUserId,
      'customer_name': customerName,
      'customer_phone': customerPhone,
      'items': items.map((item) => item.toJson()).toList(),
      'total_price': totalPrice,
      'currency': currency,
      'status': status,
      'payment_method': paymentMethod,
      'notes': notes,
    };
  }
}
```

---


## Аутентификация

### Как получить Telegram User ID (owner_tg_id)

Владелец магазина должен узнать свой Telegram User ID:

**Способ 1: Через бота**
1. Открыть Telegram
2. Найти бота `@userinfobot` или `@getidsbot`
3. Нажать Start или отправить любое сообщение
4. Бот вернёт ID (например: `123456789`)

**Способ 2: Через веб-интерфейс Telegram**
1. Открыть https://web.telegram.org
2. В URL будет ваш ID

Этот ID используется для:
- Создания магазина (`owner_tg_id` в POST /api/shops)
- Аутентификации admin операций (header `X-Owner-TG-ID`)

### services/auth_service.dart

```dart
import 'package:shared_preferences/shared_preferences.dart';

class AuthService {
  static const String _keyOwnerTgId = 'owner_tg_id';
  static const String _keyShopToken = 'shop_token';

  // Сохранить Telegram ID владельца
  Future<void> saveOwnerTgId(int ownerTgId) async {
    final prefs = await SharedPreferences.getInstance();
    await prefs.setInt(_keyOwnerTgId, ownerTgId);
  }

  // Получить Telegram ID владельца
  Future<int?> getOwnerTgId() async {
    final prefs = await SharedPreferences.getInstance();
    return prefs.getInt(_keyOwnerTgId);
  }

  // Сохранить токен магазина
  Future<void> saveShopToken(String shopToken) async {
    final prefs = await SharedPreferences.getInstance();
    await prefs.setString(_keyShopToken, shopToken);
  }

  // Получить токен магазина
  Future<String?> getShopToken() async {
    final prefs = await SharedPreferences.getInstance();
    return prefs.getString(_keyShopToken);
  }

  // Проверка авторизации
  Future<bool> isAuthenticated() async {
    final ownerTgId = await getOwnerTgId();
    final shopToken = await getShopToken();
    return ownerTgId != null && shopToken != null;
  }

  // Выход (очистка данных)
  Future<void> logout() async {
    final prefs = await SharedPreferences.getInstance();
    await prefs.remove(_keyOwnerTgId);
    await prefs.remove(_keyShopToken);
  }
}
```

---

## API Service

### services/api_service.dart

Полный HTTP клиент с retry logic, timeout и error handling:

```dart
import 'dart:convert';
import 'dart:io';
import 'package:http/http.dart' as http;
import '../config/api_config.dart';
import '../models/shop.dart';
import '../models/product.dart';
import '../models/order.dart';

class ApiException implements Exception {
  final String message;
  final int? statusCode;

  ApiException(this.message, [this.statusCode]);

  @override
  String toString() => 'ApiException: $message (status: $statusCode)';
}

class ApiService {
  final String baseUrl = ApiConfig.baseUrl;
  int? _ownerTgId;

  // Установить owner ID для headers
  void setOwnerTgId(int ownerTgId) {
    _ownerTgId = ownerTgId;
  }

  // Базовые headers
  Map<String, String> _getHeaders({bool includeAuth = false}) {
    final headers = {
      'Content-Type': 'application/json',
    };

    if (includeAuth && _ownerTgId != null) {
      headers['X-Owner-TG-ID'] = _ownerTgId.toString();
    }

    return headers;
  }

  // Retry logic с exponential backoff
  Future<http.Response> _retry(
    Future<http.Response> Function() request, {
    int maxAttempts = 3,
  }) async {
    int attempt = 0;
    while (true) {
      try {
        attempt++;
        final response = await request();
        return response;
      } catch (e) {
        if (attempt >= maxAttempts) rethrow;
        
        // Exponential backoff: 1s, 2s, 4s
        await Future.delayed(Duration(seconds: 1 << (attempt - 1)));
      }
    }
  }

  // Обработка ответа
  dynamic _handleResponse(http.Response response) {
    if (response.statusCode >= 200 && response.statusCode < 300) {
      if (response.body.isEmpty) return null;
      return json.decode(response.body);
    } else {
      final body = response.body.isNotEmpty ? json.decode(response.body) : {};
      final message = body['error'] ?? 'Request failed';
      throw ApiException(message, response.statusCode);
    }
  }

  // ==================== SHOP ENDPOINTS ====================

  // Создать магазин
  Future<Shop> createShop({
    required int ownerTgId,
    required String title,
    String? description,
    String currency = 'RUB',
    String? shopToken,
  }) async {
    final response = await _retry(() => http.post(
      Uri.parse('$baseUrl/shops'),
      headers: _getHeaders(),
      body: json.encode({
        'owner_tg_id': ownerTgId,
        'title': title,
        'description': description,
        'currency': currency,
        'shop_token': shopToken,
      }),
    ).timeout(ApiConfig.requestTimeout));

    final data = _handleResponse(response);
    return Shop.fromJson(data['shop']);
  }

  // Получить магазин по токену
  Future<Shop> getShopByToken(String shopToken) async {
    final response = await _retry(() => http.get(
      Uri.parse('$baseUrl/shops/token/$shopToken'),
      headers: _getHeaders(),
    ).timeout(ApiConfig.requestTimeout));

    final data = _handleResponse(response);
    return Shop.fromJson(data);
  }

  // Обновить магазин (требует X-Owner-TG-ID)
  Future<Shop> updateShop({
    required String shopToken,
    String? title,
    String? description,
    String? currency,
  }) async {
    final body = <String, dynamic>{};
    if (title != null) body['title'] = title;
    if (description != null) body['description'] = description;
    if (currency != null) body['currency'] = currency;

    final response = await _retry(() => http.put(
      Uri.parse('$baseUrl/shops/$shopToken'),
      headers: _getHeaders(includeAuth: true),
      body: json.encode(body),
    ).timeout(ApiConfig.requestTimeout));

    final data = _handleResponse(response);
    return Shop.fromJson(data['shop']);
  }

  // ==================== PRODUCT ENDPOINTS ====================

  // Получить все товары магазина
  Future<List<Product>> getProducts(int shopId) async {
    final response = await _retry(() => http.get(
      Uri.parse('$baseUrl/products/$shopId'),
      headers: _getHeaders(),
    ).timeout(ApiConfig.requestTimeout));

    final data = _handleResponse(response);
    final products = (data['products'] as List)
        .map((json) => Product.fromJson(json))
        .toList();
    return products;
  }

  // Создать товар (требует X-Owner-TG-ID)
  Future<Product> createProduct({
    required int shopId,
    required String title,
    required int price,
    String? imageUrl,
    String? description,
    int stockCount = 0,
    bool isActive = true,
    String? category,
    List<String>? tags,
  }) async {
    final response = await _retry(() => http.post(
      Uri.parse('$baseUrl/admin/products'),
      headers: _getHeaders(includeAuth: true),
      body: json.encode({
        'shop_id': shopId,
        'title': title,
        'price': price,
        'image_url': imageUrl,
        'description': description,
        'stock_count': stockCount,
        'is_active': isActive,
        'category': category,
        'tags': tags,
      }),
    ).timeout(ApiConfig.requestTimeout));

    final data = _handleResponse(response);
    return Product.fromJson(data['product']);
  }

  // Обновить товар (требует X-Owner-TG-ID)
  Future<Product> updateProduct({
    required int productId,
    String? title,
    int? price,
    String? imageUrl,
    String? description,
    int? stockCount,
    bool? isActive,
    String? category,
    List<String>? tags,
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

    final response = await _retry(() => http.put(
      Uri.parse('$baseUrl/admin/products/$productId'),
      headers: _getHeaders(includeAuth: true),
      body: json.encode(body),
    ).timeout(ApiConfig.requestTimeout));

    final data = _handleResponse(response);
    return Product.fromJson(data['product']);
  }

  // Удалить товар (требует X-Owner-TG-ID)
  Future<void> deleteProduct(int productId) async {
    final response = await _retry(() => http.delete(
      Uri.parse('$baseUrl/admin/products/$productId'),
      headers: _getHeaders(includeAuth: true),
    ).timeout(ApiConfig.requestTimeout));

    _handleResponse(response);
  }

  // ==================== ORDER ENDPOINTS ====================

  // Получить заказы пользователя (для просмотра своих заказов как владелец)
  // Но обычно владелец смотрит заказы СВОЕГО магазина
  Future<List<Order>> getUserOrders(int telegramUserId) async {
    final response = await _retry(() => http.get(
      Uri.parse('$baseUrl/user/orders/$telegramUserId'),
      headers: _getHeaders(),
    ).timeout(ApiConfig.requestTimeout));

    final data = _handleResponse(response);
    final orders = (data['orders'] as List)
        .map((json) => Order.fromJson(json))
        .toList();
    return orders;
  }

  // Получить заказы для конкретного магазина
  // Примечание: API возвращает все заказы пользователя, нужно фильтровать по shop_id
  Future<List<Order>> getShopOrders(int shopId, int ownerTgId) async {
    final allOrders = await getUserOrders(ownerTgId);
    return allOrders.where((order) => order.shopId == shopId).toList();
  }
}
```

---


## State Management

Используем Provider для управления состоянием приложения.

### providers/auth_provider.dart

```dart
import 'package:flutter/material.dart';
import '../services/auth_service.dart';
import '../services/api_service.dart';

class AuthProvider with ChangeNotifier {
  final AuthService _authService = AuthService();
  final ApiService _apiService = ApiService();

  int? _ownerTgId;
  String? _shopToken;
  bool _isAuthenticated = false;

  int? get ownerTgId => _ownerTgId;
  String? get shopToken => _shopToken;
  bool get isAuthenticated => _isAuthenticated;

  // Инициализация (проверка сохранённых данных)
  Future<void> init() async {
    _ownerTgId = await _authService.getOwnerTgId();
    _shopToken = await _authService.getShopToken();
    _isAuthenticated = _ownerTgId != null && _shopToken != null;
    
    if (_ownerTgId != null) {
      _apiService.setOwnerTgId(_ownerTgId!);
    }
    
    notifyListeners();
  }

  // Вход (сохранить Telegram ID)
  Future<void> login(int ownerTgId) async {
    await _authService.saveOwnerTgId(ownerTgId);
    _ownerTgId = ownerTgId;
    _apiService.setOwnerTgId(ownerTgId);
    _isAuthenticated = _shopToken != null;
    notifyListeners();
  }

  // Сохранить токен магазина
  Future<void> setShopToken(String shopToken) async {
    await _authService.saveShopToken(shopToken);
    _shopToken = shopToken;
    _isAuthenticated = _ownerTgId != null;
    notifyListeners();
  }

  // Выход
  Future<void> logout() async {
    await _authService.logout();
    _ownerTgId = null;
    _shopToken = null;
    _isAuthenticated = false;
    notifyListeners();
  }
}
```

### providers/shop_provider.dart

```dart
import 'package:flutter/material.dart';
import '../models/shop.dart';
import '../services/api_service.dart';

class ShopProvider with ChangeNotifier {
  final ApiService _apiService = ApiService();

  Shop? _shop;
  bool _isLoading = false;
  String? _error;

  Shop? get shop => _shop;
  bool get isLoading => _isLoading;
  String? get error => _error;
  bool get hasShop => _shop != null;

  // Создать магазин
  Future<void> createShop({
    required int ownerTgId,
    required String title,
    String? description,
    String currency = 'RUB',
  }) async {
    _isLoading = true;
    _error = null;
    notifyListeners();

    try {
      _shop = await _apiService.createShop(
        ownerTgId: ownerTgId,
        title: title,
        description: description,
        currency: currency,
      );
      _isLoading = false;
      notifyListeners();
    } catch (e) {
      _error = e.toString();
      _isLoading = false;
      notifyListeners();
      rethrow;
    }
  }

  // Загрузить магазин по токену
  Future<void> loadShop(String shopToken) async {
    _isLoading = true;
    _error = null;
    notifyListeners();

    try {
      _shop = await _apiService.getShopByToken(shopToken);
      _isLoading = false;
      notifyListeners();
    } catch (e) {
      _error = e.toString();
      _isLoading = false;
      notifyListeners();
      rethrow;
    }
  }

  // Обновить магазин
  Future<void> updateShop({
    required String shopToken,
    String? title,
    String? description,
    String? currency,
  }) async {
    _isLoading = true;
    _error = null;
    notifyListeners();

    try {
      _shop = await _apiService.updateShop(
        shopToken: shopToken,
        title: title,
        description: description,
        currency: currency,
      );
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

### providers/products_provider.dart

```dart
import 'package:flutter/material.dart';
import '../models/product.dart';
import '../services/api_service.dart';

class ProductsProvider with ChangeNotifier {
  final ApiService _apiService = ApiService();

  List<Product> _products = [];
  bool _isLoading = false;
  String? _error;

  List<Product> get products => _products;
  bool get isLoading => _isLoading;
  String? get error => _error;

  // Фильтры
  List<Product> get activeProducts => 
      _products.where((p) => p.isActive).toList();
  
  List<Product> get inStockProducts => 
      _products.where((p) => p.isInStock).toList();

  // Загрузить все товары
  Future<void> loadProducts(int shopId) async {
    _isLoading = true;
    _error = null;
    notifyListeners();

    try {
      _products = await _apiService.getProducts(shopId);
      _isLoading = false;
      notifyListeners();
    } catch (e) {
      _error = e.toString();
      _isLoading = false;
      notifyListeners();
      rethrow;
    }
  }

  // Создать товар
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
  }) async {
    _isLoading = true;
    _error = null;
    notifyListeners();

    try {
      final newProduct = await _apiService.createProduct(
        shopId: shopId,
        title: title,
        price: price,
        imageUrl: imageUrl,
        description: description,
        stockCount: stockCount,
        isActive: isActive,
        category: category,
        tags: tags,
      );
      
      _products.add(newProduct);
      _isLoading = false;
      notifyListeners();
    } catch (e) {
      _error = e.toString();
      _isLoading = false;
      notifyListeners();
      rethrow;
    }
  }

  // Обновить товар
  Future<void> updateProduct({
    required int productId,
    String? title,
    int? price,
    String? imageUrl,
    String? description,
    int? stockCount,
    bool? isActive,
    String? category,
    List<String>? tags,
  }) async {
    _isLoading = true;
    _error = null;
    notifyListeners();

    try {
      final updatedProduct = await _apiService.updateProduct(
        productId: productId,
        title: title,
        price: price,
        imageUrl: imageUrl,
        description: description,
        stockCount: stockCount,
        isActive: isActive,
        category: category,
        tags: tags,
      );
      
      final index = _products.indexWhere((p) => p.id == productId);
      if (index != -1) {
        _products[index] = updatedProduct;
      }
      
      _isLoading = false;
      notifyListeners();
    } catch (e) {
      _error = e.toString();
      _isLoading = false;
      notifyListeners();
      rethrow;
    }
  }

  // Удалить товар
  Future<void> deleteProduct(int productId) async {
    _isLoading = true;
    _error = null;
    notifyListeners();

    try {
      await _apiService.deleteProduct(productId);
      _products.removeWhere((p) => p.id == productId);
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

### providers/orders_provider.dart

```dart
import 'package:flutter/material.dart';
import '../models/order.dart';
import '../services/api_service.dart';

class OrdersProvider with ChangeNotifier {
  final ApiService _apiService = ApiService();

  List<Order> _orders = [];
  bool _isLoading = false;
  String? _error;

  List<Order> get orders => _orders;
  bool get isLoading => _isLoading;
  String? get error => _error;

  // Фильтры
  List<Order> get pendingOrders => 
      _orders.where((o) => o.isPending).toList();
  
  List<Order> get completedOrders => 
      _orders.where((o) => o.isCompleted).toList();
  
  List<Order> get cancelledOrders => 
      _orders.where((o) => o.isCancelled).toList();

  // Статистика
  int get totalOrders => _orders.length;
  int get totalRevenue => 
      _orders.where((o) => o.isCompleted).fold(0, (sum, o) => sum + o.totalPrice);

  // Загрузить заказы магазина
  Future<void> loadShopOrders(int shopId, int ownerTgId) async {
    _isLoading = true;
    _error = null;
    notifyListeners();

    try {
      _orders = await _apiService.getShopOrders(shopId, ownerTgId);
      _isLoading = false;
      notifyListeners();
    } catch (e) {
      _error = e.toString();
      _isLoading = false;
      notifyListeners();
      rethrow;
    }
  }

  // Обновить статус заказа (если API поддерживает)
  // Примечание: в текущем API нет endpoint для update order status
  // Но можно добавить локально для фильтрации
  void filterByStatus(String status) {
    // Implement filtering logic if needed
    notifyListeners();
  }
}
```

---


## UI Screens

### screens/auth/auth_screen.dart

Экран входа - ввод Telegram User ID:

```dart
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:provider/provider.dart';
import '../../providers/auth_provider.dart';

class AuthScreen extends StatefulWidget {
  const AuthScreen({super.key});

  @override
  State<AuthScreen> createState() => _AuthScreenState();
}

class _AuthScreenState extends State<AuthScreen> {
  final _formKey = GlobalKey<FormState>();
  final _telegramIdController = TextEditingController();
  bool _isLoading = false;

  @override
  void dispose() {
    _telegramIdController.dispose();
    super.dispose();
  }

  Future<void> _handleLogin() async {
    if (!_formKey.currentState!.validate()) return;

    setState(() => _isLoading = true);

    try {
      final telegramId = int.parse(_telegramIdController.text);
      final authProvider = context.read<AuthProvider>();
      await authProvider.login(telegramId);

      if (mounted) {
        // После входа перейти на экран настройки магазина
        Navigator.of(context).pushReplacementNamed('/shop-setup');
      }
    } catch (e) {
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text('Ошибка: $e')),
        );
      }
    } finally {
      setState(() => _isLoading = false);
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: SafeArea(
        child: Center(
          child: SingleChildScrollView(
            padding: const EdgeInsets.all(24),
            child: Form(
              key: _formKey,
              child: Column(
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  // Лого
                  Icon(
                    Icons.store,
                    size: 80,
                    color: Theme.of(context).primaryColor,
                  ),
                  const SizedBox(height: 24),
                  
                  // Заголовок
                  Text(
                    'ShopKit Admin',
                    style: Theme.of(context).textTheme.headlineMedium,
                  ),
                  const SizedBox(height: 8),
                  Text(
                    'Управление магазином',
                    style: Theme.of(context).textTheme.bodyLarge?.copyWith(
                      color: Colors.grey[600],
                    ),
                  ),
                  const SizedBox(height: 48),
                  
                  // Поле ввода Telegram ID
                  TextFormField(
                    controller: _telegramIdController,
                    decoration: const InputDecoration(
                      labelText: 'Telegram User ID',
                      hintText: 'Введите ваш Telegram ID',
                      prefixIcon: Icon(Icons.person),
                      border: OutlineInputBorder(),
                    ),
                    keyboardType: TextInputType.number,
                    inputFormatters: [FilteringTextInputFormatter.digitsOnly],
                    validator: (value) {
                      if (value == null || value.isEmpty) {
                        return 'Введите Telegram ID';
                      }
                      if (int.tryParse(value) == null) {
                        return 'ID должен быть числом';
                      }
                      return null;
                    },
                  ),
                  const SizedBox(height: 24),
                  
                  // Кнопка входа
                  SizedBox(
                    width: double.infinity,
                    child: ElevatedButton(
                      onPressed: _isLoading ? null : _handleLogin,
                      style: ElevatedButton.styleFrom(
                        padding: const EdgeInsets.symmetric(vertical: 16),
                      ),
                      child: _isLoading
                          ? const SizedBox(
                              height: 20,
                              width: 20,
                              child: CircularProgressIndicator(strokeWidth: 2),
                            )
                          : const Text('Войти'),
                    ),
                  ),
                  const SizedBox(height: 32),
                  
                  // Инструкция
                  Card(
                    child: Padding(
                      padding: const EdgeInsets.all(16),
                      child: Column(
                        crossAxisAlignment: CrossAxisAlignment.start,
                        children: [
                          Row(
                            children: [
                              Icon(Icons.info, color: Colors.blue[700]),
                              const SizedBox(width: 8),
                              const Text(
                                'Как получить Telegram ID?',
                                style: TextStyle(fontWeight: FontWeight.bold),
                              ),
                            ],
                          ),
                          const SizedBox(height: 12),
                          const Text(
                            '1. Откройте Telegram\n'
                            '2. Найдите бота @userinfobot\n'
                            '3. Отправьте ему любое сообщение\n'
                            '4. Бот вернёт ваш ID',
                            style: TextStyle(fontSize: 14),
                          ),
                        ],
                      ),
                    ),
                  ),
                ],
              ),
            ),
          ),
        ),
      ),
    );
  }
}
```

### screens/shop/shop_setup_screen.dart

Экран создания/настройки магазина:

```dart
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import '../../providers/auth_provider.dart';
import '../../providers/shop_provider.dart';

class ShopSetupScreen extends StatefulWidget {
  const ShopSetupScreen({super.key});

  @override
  State<ShopSetupScreen> createState() => _ShopSetupScreenState();
}

class _ShopSetupScreenState extends State<ShopSetupScreen> {
  final _formKey = GlobalKey<FormState>();
  final _titleController = TextEditingController();
  final _descriptionController = TextEditingController();
  String _selectedCurrency = 'RUB';
  bool _isLoading = false;

  final List<String> _currencies = ['RUB', 'USD', 'EUR', 'KZT'];

  @override
  void dispose() {
    _titleController.dispose();
    _descriptionController.dispose();
    super.dispose();
  }

  Future<void> _handleCreateShop() async {
    if (!_formKey.currentState!.validate()) return;

    setState(() => _isLoading = true);

    try {
      final authProvider = context.read<AuthProvider>();
      final shopProvider = context.read<ShopProvider>();

      await shopProvider.createShop(
        ownerTgId: authProvider.ownerTgId!,
        title: _titleController.text,
        description: _descriptionController.text.isNotEmpty
            ? _descriptionController.text
            : null,
        currency: _selectedCurrency,
      );

      // Сохранить shop token
      await authProvider.setShopToken(shopProvider.shop!.shopToken);

      if (mounted) {
        // Перейти на главный экран
        Navigator.of(context).pushReplacementNamed('/dashboard');
      }
    } catch (e) {
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text('Ошибка создания магазина: $e')),
        );
      }
    } finally {
      setState(() => _isLoading = false);
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Создание магазина'),
      ),
      body: SafeArea(
        child: SingleChildScrollView(
          padding: const EdgeInsets.all(24),
          child: Form(
            key: _formKey,
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                // Заголовок
                Text(
                  'Создайте свой магазин',
                  style: Theme.of(context).textTheme.headlineSmall,
                ),
                const SizedBox(height: 8),
                Text(
                  'Заполните основную информацию о вашем магазине',
                  style: TextStyle(color: Colors.grey[600]),
                ),
                const SizedBox(height: 32),

                // Название магазина
                TextFormField(
                  controller: _titleController,
                  decoration: const InputDecoration(
                    labelText: 'Название магазина *',
                    hintText: 'Например: "Кофейня на углу"',
                    prefixIcon: Icon(Icons.store),
                    border: OutlineInputBorder(),
                  ),
                  validator: (value) {
                    if (value == null || value.isEmpty) {
                      return 'Введите название магазина';
                    }
                    return null;
                  },
                ),
                const SizedBox(height: 16),

                // Описание
                TextFormField(
                  controller: _descriptionController,
                  decoration: const InputDecoration(
                    labelText: 'Описание',
                    hintText: 'Краткое описание вашего магазина',
                    prefixIcon: Icon(Icons.description),
                    border: OutlineInputBorder(),
                  ),
                  maxLines: 3,
                ),
                const SizedBox(height: 16),

                // Валюта
                DropdownButtonFormField<String>(
                  value: _selectedCurrency,
                  decoration: const InputDecoration(
                    labelText: 'Валюта',
                    prefixIcon: Icon(Icons.monetization_on),
                    border: OutlineInputBorder(),
                  ),
                  items: _currencies.map((currency) {
                    return DropdownMenuItem(
                      value: currency,
                      child: Text(currency),
                    );
                  }).toList(),
                  onChanged: (value) {
                    if (value != null) {
                      setState(() => _selectedCurrency = value);
                    }
                  },
                ),
                const SizedBox(height: 32),

                // Кнопка создания
                SizedBox(
                  width: double.infinity,
                  child: ElevatedButton(
                    onPressed: _isLoading ? null : _handleCreateShop,
                    style: ElevatedButton.styleFrom(
                      padding: const EdgeInsets.symmetric(vertical: 16),
                    ),
                    child: _isLoading
                        ? const SizedBox(
                            height: 20,
                            width: 20,
                            child: CircularProgressIndicator(strokeWidth: 2),
                          )
                        : const Text('Создать магазин'),
                  ),
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }
}
```

### screens/dashboard/dashboard_screen.dart

Главный экран со статистикой:

```dart
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:intl/intl.dart';
import '../../providers/shop_provider.dart';
import '../../providers/products_provider.dart';
import '../../providers/orders_provider.dart';
import '../../providers/auth_provider.dart';

class DashboardScreen extends StatefulWidget {
  const DashboardScreen({super.key});

  @override
  State<DashboardScreen> createState() => _DashboardScreenState();
}

class _DashboardScreenState extends State<DashboardScreen> {
  bool _isLoading = true;

  @override
  void initState() {
    super.initState();
    _loadData();
  }

  Future<void> _loadData() async {
    setState(() => _isLoading = true);

    try {
      final authProvider = context.read<AuthProvider>();
      final shopProvider = context.read<ShopProvider>();
      final productsProvider = context.read<ProductsProvider>();
      final ordersProvider = context.read<OrdersProvider>();

      // Загрузить магазин
      if (authProvider.shopToken != null) {
        await shopProvider.loadShop(authProvider.shopToken!);
        
        // Загрузить товары и заказы
        if (shopProvider.shop != null) {
          await Future.wait([
            productsProvider.loadProducts(shopProvider.shop!.id),
            ordersProvider.loadShopOrders(
              shopProvider.shop!.id,
              authProvider.ownerTgId!,
            ),
          ]);
        }
      }
    } catch (e) {
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text('Ошибка загрузки: $e')),
        );
      }
    } finally {
      setState(() => _isLoading = false);
    }
  }

  @override
  Widget build(BuildContext context) {
    final shopProvider = context.watch<ShopProvider>();
    final productsProvider = context.watch<ProductsProvider>();
    final ordersProvider = context.watch<OrdersProvider>();

    return Scaffold(
      appBar: AppBar(
        title: const Text('ShopKit Admin'),
        actions: [
          IconButton(
            icon: const Icon(Icons.settings),
            onPressed: () => Navigator.pushNamed(context, '/settings'),
          ),
        ],
      ),
      body: _isLoading
          ? const Center(child: CircularProgressIndicator())
          : RefreshIndicator(
              onRefresh: _loadData,
              child: SingleChildScrollView(
                physics: const AlwaysScrollableScrollPhysics(),
                padding: const EdgeInsets.all(16),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    // Информация о магазине
                    if (shopProvider.shop != null) ...[
                      _ShopInfoCard(shop: shopProvider.shop!),
                      const SizedBox(height: 16),
                    ],

                    // Статистика
                    Text(
                      'Статистика',
                      style: Theme.of(context).textTheme.titleLarge,
                    ),
                    const SizedBox(height: 16),

                    // Карточки статистики
                    GridView.count(
                      crossAxisCount: 2,
                      crossAxisSpacing: 16,
                      mainAxisSpacing: 16,
                      shrinkWrap: true,
                      physics: const NeverScrollableScrollPhysics(),
                      childAspectRatio: 1.5,
                      children: [
                        _StatCard(
                          title: 'Товары',
                          value: productsProvider.products.length.toString(),
                          icon: Icons.inventory,
                          color: Colors.blue,
                          onTap: () => Navigator.pushNamed(context, '/products'),
                        ),
                        _StatCard(
                          title: 'Заказы',
                          value: ordersProvider.totalOrders.toString(),
                          icon: Icons.shopping_bag,
                          color: Colors.green,
                          onTap: () => Navigator.pushNamed(context, '/orders'),
                        ),
                        _StatCard(
                          title: 'Ожидают',
                          value: ordersProvider.pendingOrders.length.toString(),
                          icon: Icons.pending,
                          color: Colors.orange,
                          onTap: () => Navigator.pushNamed(context, '/orders'),
                        ),
                        _StatCard(
                          title: 'Выручка',
                          value: _formatRevenue(
                            ordersProvider.totalRevenue,
                            shopProvider.shop?.currency ?? 'RUB',
                          ),
                          icon: Icons.attach_money,
                          color: Colors.purple,
                        ),
                      ],
                    ),
                    const SizedBox(height: 24),

                    // Быстрые действия
                    Text(
                      'Быстрые действия',
                      style: Theme.of(context).textTheme.titleLarge,
                    ),
                    const SizedBox(height: 16),
                    _ActionButton(
                      icon: Icons.add_business,
                      label: 'Добавить товар',
                      onTap: () => Navigator.pushNamed(context, '/product-form'),
                    ),
                    const SizedBox(height: 8),
                    _ActionButton(
                      icon: Icons.list,
                      label: 'Все товары',
                      onTap: () => Navigator.pushNamed(context, '/products'),
                    ),
                    const SizedBox(height: 8),
                    _ActionButton(
                      icon: Icons.shopping_cart,
                      label: 'Заказы',
                      onTap: () => Navigator.pushNamed(context, '/orders'),
                    ),
                  ],
                ),
              ),
            ),
    );
  }

  String _formatRevenue(int totalKopeks, String currency) {
    final rubles = totalKopeks / 100;
    return '${NumberFormat('#,##0.00').format(rubles)} $currency';
  }
}

// Карточка информации о магазине
class _ShopInfoCard extends StatelessWidget {
  final dynamic shop;

  const _ShopInfoCard({required this.shop});

  @override
  Widget build(BuildContext context) {
    return Card(
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Row(
              children: [
                const Icon(Icons.store, size: 32),
                const SizedBox(width: 12),
                Expanded(
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Text(
                        shop.title,
                        style: Theme.of(context).textTheme.titleLarge,
                      ),
                      if (shop.description != null)
                        Text(
                          shop.description!,
                          style: TextStyle(color: Colors.grey[600]),
                        ),
                    ],
                  ),
                ),
              ],
            ),
            const SizedBox(height: 12),
            Row(
              children: [
                Chip(
                  label: Text('Token: ${shop.shopToken}'),
                  avatar: const Icon(Icons.key, size: 16),
                ),
              ],
            ),
          ],
        ),
      ),
    );
  }
}

// Карточка статистики
class _StatCard extends StatelessWidget {
  final String title;
  final String value;
  final IconData icon;
  final Color color;
  final VoidCallback? onTap;

  const _StatCard({
    required this.title,
    required this.value,
    required this.icon,
    required this.color,
    this.onTap,
  });

  @override
  Widget build(BuildContext context) {
    return Card(
      child: InkWell(
        onTap: onTap,
        borderRadius: BorderRadius.circular(8),
        child: Padding(
          padding: const EdgeInsets.all(16),
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Icon(icon, size: 32, color: color),
              const SizedBox(height: 8),
              Text(
                value,
                style: Theme.of(context).textTheme.headlineSmall?.copyWith(
                      color: color,
                      fontWeight: FontWeight.bold,
                    ),
              ),
              Text(
                title,
                style: TextStyle(color: Colors.grey[600]),
                textAlign: TextAlign.center,
              ),
            ],
          ),
        ),
      ),
    );
  }
}

// Кнопка действия
class _ActionButton extends StatelessWidget {
  final IconData icon;
  final String label;
  final VoidCallback onTap;

  const _ActionButton({
    required this.icon,
    required this.label,
    required this.onTap,
  });

  @override
  Widget build(BuildContext context) {
    return Card(
      child: ListTile(
        leading: Icon(icon),
        title: Text(label),
        trailing: const Icon(Icons.chevron_right),
        onTap: onTap,
      ),
    );
  }
}
```


### screens/products/products_list_screen.dart

Список всех товаров магазина:

```dart
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import '../../providers/products_provider.dart';
import '../../models/product.dart';

class ProductsListScreen extends StatelessWidget {
  const ProductsListScreen({super.key});

  @override
  Widget build(BuildContext context) {
    final productsProvider = context.watch<ProductsProvider>();

    return Scaffold(
      appBar: AppBar(
        title: const Text('Товары'),
        actions: [
          IconButton(
            icon: const Icon(Icons.add),
            onPressed: () => Navigator.pushNamed(context, '/product-form'),
          ),
        ],
      ),
      body: productsProvider.isLoading
          ? const Center(child: CircularProgressIndicator())
          : productsProvider.products.isEmpty
              ? Center(
                  child: Column(
                    mainAxisAlignment: MainAxisAlignment.center,
                    children: [
                      const Icon(Icons.inventory_2_outlined, size: 64, color: Colors.grey),
                      const SizedBox(height: 16),
                      const Text('Нет товаров'),
                      const SizedBox(height: 8),
                      ElevatedButton.icon(
                        onPressed: () => Navigator.pushNamed(context, '/product-form'),
                        icon: const Icon(Icons.add),
                        label: const Text('Добавить товар'),
                      ),
                    ],
                  ),
                )
              : ListView.builder(
                  padding: const EdgeInsets.all(16),
                  itemCount: productsProvider.products.length,
                  itemBuilder: (context, index) {
                    final product = productsProvider.products[index];
                    return _ProductCard(product: product);
                  },
                ),
    );
  }
}

class _ProductCard extends StatelessWidget {
  final Product product;

  const _ProductCard({required this.product});

  @override
  Widget build(BuildContext context) {
    return Card(
      margin: const EdgeInsets.only(bottom: 12),
      child: InkWell(
        onTap: () => Navigator.pushNamed(
          context,
          '/product-detail',
          arguments: product,
        ),
        child: Padding(
          padding: const EdgeInsets.all(12),
          child: Row(
            children: [
              // Изображение товара
              ClipRRect(
                borderRadius: BorderRadius.circular(8),
                child: product.imageUrl != null
                    ? Image.network(
                        product.imageUrl!,
                        width: 80,
                        height: 80,
                        fit: BoxFit.cover,
                        errorBuilder: (_, __, ___) => _PlaceholderImage(),
                      )
                    : _PlaceholderImage(),
              ),
              const SizedBox(width: 12),

              // Информация о товаре
              Expanded(
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Text(
                      product.title,
                      style: const TextStyle(
                        fontSize: 16,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                    const SizedBox(height: 4),
                    Text(
                      product.formattedPrice,
                      style: TextStyle(
                        fontSize: 14,
                        color: Colors.green[700],
                        fontWeight: FontWeight.w600,
                      ),
                    ),
                    const SizedBox(height: 8),
                    Row(
                      children: [
                        // Остаток
                        Chip(
                          label: Text('Остаток: ${product.stockCount}'),
                          visualDensity: VisualDensity.compact,
                          backgroundColor: product.isInStock
                              ? Colors.green[100]
                              : Colors.red[100],
                        ),
                        const SizedBox(width: 8),
                        // Статус
                        Icon(
                          product.isActive ? Icons.check_circle : Icons.cancel,
                          size: 16,
                          color: product.isActive ? Colors.green : Colors.grey,
                        ),
                        const SizedBox(width: 4),
                        Text(
                          product.isActive ? 'Активен' : 'Неактивен',
                          style: const TextStyle(fontSize: 12),
                        ),
                      ],
                    ),
                  ],
                ),
              ),

              // Кнопка редактирования
              IconButton(
                icon: const Icon(Icons.edit),
                onPressed: () => Navigator.pushNamed(
                  context,
                  '/product-form',
                  arguments: product,
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }
}

class _PlaceholderImage extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return Container(
      width: 80,
      height: 80,
      color: Colors.grey[300],
      child: const Icon(Icons.image, size: 40, color: Colors.grey),
    );
  }
}
```

### screens/products/product_form_screen.dart

Форма добавления/редактирования товара:

```dart
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:provider/provider.dart';
import 'package:image_picker/image_picker.dart';
import 'dart:io';
import '../../providers/products_provider.dart';
import '../../providers/shop_provider.dart';
import '../../models/product.dart';

class ProductFormScreen extends StatefulWidget {
  final Product? product; // Если null - создание, иначе редактирование

  const ProductFormScreen({super.key, this.product});

  @override
  State<ProductFormScreen> createState() => _ProductFormScreenState();
}

class _ProductFormScreenState extends State<ProductFormScreen> {
  final _formKey = GlobalKey<FormState>();
  final _titleController = TextEditingController();
  final _priceController = TextEditingController();
  final _descriptionController = TextEditingController();
  final _stockController = TextEditingController();
  final _categoryController = TextEditingController();
  final _imageUrlController = TextEditingController();

  bool _isActive = true;
  bool _isLoading = false;
  File? _imageFile;

  bool get _isEditing => widget.product != null;

  @override
  void initState() {
    super.initState();
    if (_isEditing) {
      _titleController.text = widget.product!.title;
      _priceController.text = (widget.product!.price / 100).toStringAsFixed(2);
      _descriptionController.text = widget.product!.description ?? '';
      _stockController.text = widget.product!.stockCount.toString();
      _categoryController.text = widget.product!.category ?? '';
      _imageUrlController.text = widget.product!.imageUrl ?? '';
      _isActive = widget.product!.isActive;
    }
  }

  @override
  void dispose() {
    _titleController.dispose();
    _priceController.dispose();
    _descriptionController.dispose();
    _stockController.dispose();
    _categoryController.dispose();
    _imageUrlController.dispose();
    super.dispose();
  }

  Future<void> _pickImage() async {
    final picker = ImagePicker();
    final pickedFile = await picker.pickImage(source: ImageSource.gallery);
    
    if (pickedFile != null) {
      setState(() {
        _imageFile = File(pickedFile.path);
        // В реальном приложении нужно загрузить на сервер
        // Сейчас просто показываем локальный файл
      });
    }
  }

  Future<void> _handleSave() async {
    if (!_formKey.currentState!.validate()) return;

    setState(() => _isLoading = true);

    try {
      final shopProvider = context.read<ShopProvider>();
      final productsProvider = context.read<ProductsProvider>();

      // Конвертировать цену в копейки
      final priceInKopeks = (double.parse(_priceController.text) * 100).toInt();

      if (_isEditing) {
        // Обновление
        await productsProvider.updateProduct(
          productId: widget.product!.id,
          title: _titleController.text,
          price: priceInKopeks,
          description: _descriptionController.text.isNotEmpty
              ? _descriptionController.text
              : null,
          stockCount: int.parse(_stockController.text),
          isActive: _isActive,
          category: _categoryController.text.isNotEmpty
              ? _categoryController.text
              : null,
          imageUrl: _imageUrlController.text.isNotEmpty
              ? _imageUrlController.text
              : null,
        );
      } else {
        // Создание
        await productsProvider.createProduct(
          shopId: shopProvider.shop!.id,
          title: _titleController.text,
          price: priceInKopeks,
          description: _descriptionController.text.isNotEmpty
              ? _descriptionController.text
              : null,
          stockCount: int.parse(_stockController.text),
          isActive: _isActive,
          category: _categoryController.text.isNotEmpty
              ? _categoryController.text
              : null,
          imageUrl: _imageUrlController.text.isNotEmpty
              ? _imageUrlController.text
              : null,
        );
      }

      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text(_isEditing ? 'Товар обновлён' : 'Товар создан')),
        );
        Navigator.pop(context);
      }
    } catch (e) {
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text('Ошибка: $e')),
        );
      }
    } finally {
      setState(() => _isLoading = false);
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(_isEditing ? 'Редактировать товар' : 'Добавить товар'),
      ),
      body: SingleChildScrollView(
        padding: const EdgeInsets.all(16),
        child: Form(
          key: _formKey,
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              // Название
              TextFormField(
                controller: _titleController,
                decoration: const InputDecoration(
                  labelText: 'Название товара *',
                  border: OutlineInputBorder(),
                ),
                validator: (value) {
                  if (value == null || value.isEmpty) {
                    return 'Введите название';
                  }
                  return null;
                },
              ),
              const SizedBox(height: 16),

              // Цена
              TextFormField(
                controller: _priceController,
                decoration: const InputDecoration(
                  labelText: 'Цена *',
                  hintText: '100.00',
                  border: OutlineInputBorder(),
                  suffixText: 'RUB',
                ),
                keyboardType: const TextInputType.numberWithOptions(decimal: true),
                inputFormatters: [
                  FilteringTextInputFormatter.allow(RegExp(r'^\d+\.?\d{0,2}')),
                ],
                validator: (value) {
                  if (value == null || value.isEmpty) {
                    return 'Введите цену';
                  }
                  if (double.tryParse(value) == null) {
                    return 'Неверный формат цены';
                  }
                  return null;
                },
              ),
              const SizedBox(height: 16),

              // Остаток
              TextFormField(
                controller: _stockController,
                decoration: const InputDecoration(
                  labelText: 'Остаток на складе *',
                  border: OutlineInputBorder(),
                ),
                keyboardType: TextInputType.number,
                inputFormatters: [FilteringTextInputFormatter.digitsOnly],
                validator: (value) {
                  if (value == null || value.isEmpty) {
                    return 'Введите количество';
                  }
                  return null;
                },
              ),
              const SizedBox(height: 16),

              // Категория
              TextFormField(
                controller: _categoryController,
                decoration: const InputDecoration(
                  labelText: 'Категория',
                  hintText: 'Например: Кофе, Чай',
                  border: OutlineInputBorder(),
                ),
              ),
              const SizedBox(height: 16),

              // Описание
              TextFormField(
                controller: _descriptionController,
                decoration: const InputDecoration(
                  labelText: 'Описание',
                  hintText: 'Подробное описание товара',
                  border: OutlineInputBorder(),
                ),
                maxLines: 4,
              ),
              const SizedBox(height: 16),

              // URL изображения
              TextFormField(
                controller: _imageUrlController,
                decoration: const InputDecoration(
                  labelText: 'URL изображения',
                  hintText: 'https://example.com/image.jpg',
                  border: OutlineInputBorder(),
                ),
              ),
              const SizedBox(height: 8),

              // Кнопка выбора изображения
              OutlinedButton.icon(
                onPressed: _pickImage,
                icon: const Icon(Icons.image),
                label: const Text('Выбрать изображение'),
              ),
              
              if (_imageFile != null) ...[
                const SizedBox(height: 8),
                Image.file(_imageFile!, height: 200),
                const Text(
                  'Примечание: Для загрузки изображения на сервер нужна реализация upload endpoint',
                  style: TextStyle(fontSize: 12, color: Colors.orange),
                ),
              ],
              const SizedBox(height: 16),

              // Статус активности
              SwitchListTile(
                title: const Text('Товар активен'),
                subtitle: const Text('Виден покупателям'),
                value: _isActive,
                onChanged: (value) => setState(() => _isActive = value),
              ),
              const SizedBox(height: 24),

              // Кнопки
              Row(
                children: [
                  Expanded(
                    child: OutlinedButton(
                      onPressed: _isLoading ? null : () => Navigator.pop(context),
                      child: const Text('Отмена'),
                    ),
                  ),
                  const SizedBox(width: 16),
                  Expanded(
                    child: ElevatedButton(
                      onPressed: _isLoading ? null : _handleSave,
                      child: _isLoading
                          ? const SizedBox(
                              height: 20,
                              width: 20,
                              child: CircularProgressIndicator(strokeWidth: 2),
                            )
                          : Text(_isEditing ? 'Сохранить' : 'Создать'),
                    ),
                  ),
                ],
              ),
            ],
          ),
        ),
      ),
    );
  }
}
```


### screens/orders/orders_list_screen.dart

Список заказов клиентов:

```dart
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:intl/intl.dart';
import '../../providers/orders_provider.dart';
import '../../models/order.dart';

class OrdersListScreen extends StatelessWidget {
  const OrdersListScreen({super.key});

  @override
  Widget build(BuildContext context) {
    final ordersProvider = context.watch<OrdersProvider>();

    return Scaffold(
      appBar: AppBar(
        title: const Text('Заказы'),
      ),
      body: ordersProvider.isLoading
          ? const Center(child: CircularProgressIndicator())
          : ordersProvider.orders.isEmpty
              ? const Center(
                  child: Column(
                    mainAxisAlignment: MainAxisAlignment.center,
                    children: [
                      Icon(Icons.shopping_bag_outlined, size: 64, color: Colors.grey),
                      SizedBox(height: 16),
                      Text('Нет заказов'),
                    ],
                  ),
                )
              : ListView.builder(
                  padding: const EdgeInsets.all(16),
                  itemCount: ordersProvider.orders.length,
                  itemBuilder: (context, index) {
                    final order = ordersProvider.orders[index];
                    return _OrderCard(order: order);
                  },
                ),
    );
  }
}

class _OrderCard extends StatelessWidget {
  final Order order;

  const _OrderCard({required this.order});

  @override
  Widget build(BuildContext context) {
    return Card(
      margin: const EdgeInsets.only(bottom: 12),
      child: InkWell(
        onTap: () => Navigator.pushNamed(
          context,
          '/order-detail',
          arguments: order,
        ),
        child: Padding(
          padding: const EdgeInsets.all(16),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              // Заголовок с номером заказа
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  Text(
                    'Заказ #${order.id}',
                    style: const TextStyle(
                      fontSize: 16,
                      fontWeight: FontWeight.bold,
                    ),
                  ),
                  _StatusChip(status: order.status),
                ],
              ),
              const SizedBox(height: 12),

              // Информация о клиенте
              Row(
                children: [
                  const Icon(Icons.person, size: 16, color: Colors.grey),
                  const SizedBox(width: 8),
                  Text(order.customerName),
                ],
              ),
              const SizedBox(height: 4),

              // Дата
              Row(
                children: [
                  const Icon(Icons.calendar_today, size: 16, color: Colors.grey),
                  const SizedBox(width: 8),
                  Text(DateFormat('dd.MM.yyyy HH:mm').format(order.createdAt)),
                ],
              ),
              const SizedBox(height: 12),

              // Итого
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  Text(
                    '${order.itemsCount} товаров',
                    style: const TextStyle(color: Colors.grey),
                  ),
                  Text(
                    order.formattedTotalPrice,
                    style: TextStyle(
                      fontSize: 18,
                      fontWeight: FontWeight.bold,
                      color: Colors.green[700],
                    ),
                  ),
                ],
              ),
            ],
          ),
        ),
      ),
    );
  }
}

class _StatusChip extends StatelessWidget {
  final String status;

  const _StatusChip({required this.status});

  @override
  Widget build(BuildContext context) {
    Color color;
    String label;

    switch (status) {
      case 'pending':
        color = Colors.orange;
        label = 'Ожидает';
        break;
      case 'completed':
        color = Colors.green;
        label = 'Выполнен';
        break;
      case 'cancelled':
        color = Colors.red;
        label = 'Отменён';
        break;
      default:
        color = Colors.grey;
        label = status;
    }

    return Chip(
      label: Text(label, style: const TextStyle(fontSize: 12)),
      backgroundColor: color.withOpacity(0.2),
      labelStyle: TextStyle(color: color[700]),
      visualDensity: VisualDensity.compact,
    );
  }
}
```

### screens/orders/order_detail_screen.dart

Детали заказа:

```dart
import 'package:flutter/material.dart';
import 'package:intl/intl.dart';
import '../../models/order.dart';

class OrderDetailScreen extends StatelessWidget {
  final Order order;

  const OrderDetailScreen({super.key, required this.order});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text('Заказ #${order.id}'),
      ),
      body: SingleChildScrollView(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            // Статус
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceBetween,
              children: [
                Text(
                  'Статус',
                  style: Theme.of(context).textTheme.titleMedium,
                ),
                _StatusChip(status: order.status),
              ],
            ),
            const Divider(height: 32),

            // Информация о клиенте
            Text(
              'Клиент',
              style: Theme.of(context).textTheme.titleMedium,
            ),
            const SizedBox(height: 8),
            _InfoRow(icon: Icons.person, label: order.customerName),
            if (order.customerPhone != null)
              _InfoRow(icon: Icons.phone, label: order.customerPhone!),
            _InfoRow(
              icon: Icons.telegram,
              label: 'ID: ${order.telegramUserId}',
            ),
            const Divider(height: 32),

            // Дата и время
            Text(
              'Дата заказа',
              style: Theme.of(context).textTheme.titleMedium,
            ),
            const SizedBox(height: 8),
            _InfoRow(
              icon: Icons.calendar_today,
              label: DateFormat('dd.MM.yyyy HH:mm').format(order.createdAt),
            ),
            const Divider(height: 32),

            // Товары
            Text(
              'Товары',
              style: Theme.of(context).textTheme.titleMedium,
            ),
            const SizedBox(height: 12),
            ...order.items.map((item) => _OrderItemCard(item: item)),
            const Divider(height: 32),

            // Итого
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceBetween,
              children: [
                Text(
                  'Итого:',
                  style: Theme.of(context).textTheme.titleLarge,
                ),
                Text(
                  order.formattedTotalPrice,
                  style: Theme.of(context).textTheme.titleLarge?.copyWith(
                        color: Colors.green[700],
                        fontWeight: FontWeight.bold,
                      ),
                ),
              ],
            ),

            // Примечания
            if (order.notes != null) ...[
              const Divider(height: 32),
              Text(
                'Примечания',
                style: Theme.of(context).textTheme.titleMedium,
              ),
              const SizedBox(height: 8),
              Text(order.notes!),
            ],
          ],
        ),
      ),
    );
  }
}

class _InfoRow extends StatelessWidget {
  final IconData icon;
  final String label;

  const _InfoRow({required this.icon, required this.label});

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.only(bottom: 8),
      child: Row(
        children: [
          Icon(icon, size: 16, color: Colors.grey),
          const SizedBox(width: 12),
          Expanded(child: Text(label)),
        ],
      ),
    );
  }
}

class _OrderItemCard extends StatelessWidget {
  final dynamic item;

  const _OrderItemCard({required this.item});

  @override
  Widget build(BuildContext context) {
    final totalPrice = item.quantity * item.price;
    final formattedPrice = (totalPrice / 100).toStringAsFixed(2);

    return Card(
      margin: const EdgeInsets.only(bottom: 8),
      child: Padding(
        padding: const EdgeInsets.all(12),
        child: Row(
          children: [
            // Количество
            Container(
              width: 40,
              height: 40,
              decoration: BoxDecoration(
                color: Colors.blue[100],
                borderRadius: BorderRadius.circular(8),
              ),
              child: Center(
                child: Text(
                  '${item.quantity}x',
                  style: TextStyle(
                    color: Colors.blue[700],
                    fontWeight: FontWeight.bold,
                  ),
                ),
              ),
            ),
            const SizedBox(width: 12),

            // Название
            Expanded(
              child: Text(
                item.title,
                style: const TextStyle(fontSize: 14),
              ),
            ),

            // Цена
            Text(
              '$formattedPrice RUB',
              style: TextStyle(
                fontSize: 14,
                fontWeight: FontWeight.bold,
                color: Colors.green[700],
              ),
            ),
          ],
        ),
      ),
    );
  }
}

class _StatusChip extends StatelessWidget {
  final String status;

  const _StatusChip({required this.status});

  @override
  Widget build(BuildContext context) {
    Color color;
    String label;

    switch (status) {
      case 'pending':
        color = Colors.orange;
        label = 'Ожидает';
        break;
      case 'completed':
        color = Colors.green;
        label = 'Выполнен';
        break;
      case 'cancelled':
        color = Colors.red;
        label = 'Отменён';
        break;
      default:
        color = Colors.grey;
        label = status;
    }

    return Chip(
      label: Text(label),
      backgroundColor: color.withOpacity(0.2),
      labelStyle: TextStyle(color: color[700]),
    );
  }
}
```

---

## Навигация

### main.dart

Главный файл приложения с настройкой роутинга:

```dart
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

import 'config/api_config.dart';
import 'providers/auth_provider.dart';
import 'providers/shop_provider.dart';
import 'providers/products_provider.dart';
import 'providers/orders_provider.dart';
import 'screens/auth/auth_screen.dart';
import 'screens/shop/shop_setup_screen.dart';
import 'screens/dashboard/dashboard_screen.dart';
import 'screens/products/products_list_screen.dart';
import 'screens/products/product_form_screen.dart';
import 'screens/orders/orders_list_screen.dart';
import 'screens/orders/order_detail_screen.dart';
import 'models/product.dart';
import 'models/order.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MultiProvider(
      providers: [
        ChangeNotifierProvider(create: (_) => AuthProvider()..init()),
        ChangeNotifierProvider(create: (_) => ShopProvider()),
        ChangeNotifierProvider(create: (_) => ProductsProvider()),
        ChangeNotifierProvider(create: (_) => OrdersProvider()),
      ],
      child: MaterialApp(
        title: 'ShopKit Admin',
        debugShowCheckedModeBanner: false,
        theme: ThemeData(
          primarySwatch: Colors.blue,
          useMaterial3: true,
        ),
        home: const AuthScreen(),
        onGenerateRoute: (settings) {
          switch (settings.name) {
            case '/auth':
              return MaterialPageRoute(builder: (_) => const AuthScreen());
            
            case '/shop-setup':
              return MaterialPageRoute(builder: (_) => const ShopSetupScreen());
            
            case '/dashboard':
              return MaterialPageRoute(builder: (_) => const DashboardScreen());
            
            case '/products':
              return MaterialPageRoute(builder: (_) => const ProductsListScreen());
            
            case '/product-form':
              final product = settings.arguments as Product?;
              return MaterialPageRoute(
                builder: (_) => ProductFormScreen(product: product),
              );
            
            case '/orders':
              return MaterialPageRoute(builder: (_) => const OrdersListScreen());
            
            case '/order-detail':
              final order = settings.arguments as Order;
              return MaterialPageRoute(
                builder: (_) => OrderDetailScreen(order: order),
              );
            
            default:
              return MaterialPageRoute(builder: (_) => const AuthScreen());
          }
        },
      ),
    );
  }
}
```

---

## Загрузка изображений

### Примечание

В текущей версии API нет endpoint для загрузки изображений. Есть два варианта:

**Вариант 1: Использовать внешний сервис (рекомендуется)**

Загружать изображения на Cloudinary, AWS S3, или другой CDN, затем передавать URL в API.

```dart
// Пример с Cloudinary
Future<String> uploadToCloudinary(File imageFile) async {
  final url = Uri.parse('https://api.cloudinary.com/v1_1/YOUR_CLOUD/image/upload');
  
  var request = http.MultipartRequest('POST', url);
  request.fields['upload_preset'] = 'YOUR_PRESET';
  request.files.add(await http.MultipartFile.fromPath('file', imageFile.path));
  
  final response = await request.send();
  final responseData = await response.stream.toBytes();
  final result = json.decode(String.fromCharCodes(responseData));
  
  return result['secure_url'];
}
```

**Вариант 2: Добавить endpoint в backend**

Если нужен upload на сервер, добавьте endpoint в C++ backend:

```cpp
// POST /api/admin/upload-image
// Принимает multipart/form-data
// Возвращает { "url": "https://..." }
```

---


## Тестирование

### Unit тесты для моделей

```dart
// test/models/product_test.dart
import 'package:flutter_test/flutter_test.dart';
import 'package:shopkit_admin/models/product.dart';

void main() {
  group('Product Model', () {
    test('fromJson создаёт Product корректно', () {
      final json = {
        'id': 1,
        'product_id': 'uuid-123',
        'shop_id': 1,
        'title': 'Test Product',
        'price': 10000,
        'currency': 'RUB',
        'stock_count': 50,
        'is_active': true,
        'created_at': '2024-01-01T00:00:00',
        'updated_at': '2024-01-01T00:00:00',
      };

      final product = Product.fromJson(json);

      expect(product.title, 'Test Product');
      expect(product.price, 10000);
      expect(product.formattedPrice, '100.00 RUB');
      expect(product.isInStock, true);
    });

    test('formattedPrice форматирует цену правильно', () {
      final product = Product(
        id: 1,
        productId: 'uuid',
        shopId: 1,
        title: 'Test',
        price: 12345,
        currency: 'RUB',
        stockCount: 10,
        isActive: true,
        createdAt: DateTime.now(),
        updatedAt: DateTime.now(),
      );

      expect(product.formattedPrice, '123.45 RUB');
    });
  });
}
```

### Widget тесты

```dart
// test/widgets/product_card_test.dart
import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:shopkit_admin/models/product.dart';

void main() {
  testWidgets('ProductCard показывает информацию о товаре', (tester) async {
    final product = Product(
      id: 1,
      productId: 'uuid',
      shopId: 1,
      title: 'Test Product',
      price: 10000,
      currency: 'RUB',
      stockCount: 5,
      isActive: true,
      createdAt: DateTime.now(),
      updatedAt: DateTime.now(),
    );

    await tester.pumpWidget(
      MaterialApp(
        home: Scaffold(
          body: Text(product.title), // Упрощённо для примера
        ),
      ),
    );

    expect(find.text('Test Product'), findsOneWidget);
  });
}
```

---

## Деплой

### Android

#### 1. Настройка build.gradle

`android/app/build.gradle`:

```gradle
android {
    defaultConfig {
        applicationId "com.yourcompany.shopkit_admin"
        minSdkVersion 21
        targetSdkVersion 33
        versionCode 1
        versionName "1.0.0"
    }

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
            minifyEnabled true
            shrinkResources true
        }
    }
}
```

#### 2. Создать keystore

```bash
keytool -genkey -v -keystore ~/shopkit-admin-key.jks \
  -keyalg RSA -keysize 2048 -validity 10000 \
  -alias shopkit-admin
```

#### 3. Создать key.properties

`android/key.properties`:

```properties
storePassword=YOUR_STORE_PASSWORD
keyPassword=YOUR_KEY_PASSWORD
keyAlias=shopkit-admin
storeFile=/path/to/shopkit-admin-key.jks
```

#### 4. Собрать APK/AAB

```bash
# APK для тестирования
flutter build apk --release

# AAB для Play Store
flutter build appbundle --release
```

#### 5. Загрузить в Google Play Console

1. Зайти на https://play.google.com/console
2. Создать новое приложение
3. Загрузить AAB файл
4. Заполнить информацию о приложении
5. Опубликовать

### iOS

#### 1. Настройка Xcode

```bash
cd ios
pod install
open Runner.xcworkspace
```

#### 2. Настроить Bundle Identifier

В Xcode:
- Runner → General → Identity
- Bundle Identifier: `com.yourcompany.shopkit-admin`

#### 3. Настроить Signing

- Signing & Capabilities
- Team: выбрать свою команду
- Automatically manage signing

#### 4. Собрать IPA

```bash
flutter build ios --release
```

Или через Xcode:
- Product → Archive
- Distribute App → App Store Connect

#### 5. Загрузить в App Store Connect

1. Зайти на https://appstoreconnect.apple.com
2. My Apps → Create New App
3. Загрузить build через Xcode или Transporter
4. Заполнить метаданные
5. Submit for Review

---

## Platform-specific конфигурации

### Android Permissions

`android/app/src/main/AndroidManifest.xml`:

```xml
<manifest>
    <!-- Доступ в интернет -->
    <uses-permission android:name="android.permission.INTERNET" />
    
    <!-- Доступ к камере для фото товаров -->
    <uses-permission android:name="android.permission.CAMERA" />
    
    <!-- Доступ к галерее -->
    <uses-permission android:name="android.permission.READ_EXTERNAL_STORAGE" />
    <uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE"
        android:maxSdkVersion="28" />

    <application
        android:label="ShopKit Admin"
        android:icon="@mipmap/ic_launcher"
        android:usesCleartextTraffic="true"> <!-- Если используете HTTP -->
        
        <activity android:name=".MainActivity">
            <!-- Deep links для уведомлений -->
            <intent-filter>
                <action android:name="android.intent.action.VIEW" />
                <category android:name="android.intent.category.DEFAULT" />
                <category android:name="android.intent.category.BROWSABLE" />
                <data android:scheme="shopkit" android:host="admin" />
            </intent-filter>
        </activity>
    </application>
</manifest>
```

### iOS Permissions

`ios/Runner/Info.plist`:

```xml
<dict>
    <!-- Доступ к камере -->
    <key>NSCameraUsageDescription</key>
    <string>Требуется для фотографирования товаров</string>
    
    <!-- Доступ к галерее -->
    <key>NSPhotoLibraryUsageDescription</key>
    <string>Требуется для выбора фото товаров</string>
    
    <!-- Интернет -->
    <key>NSAppTransportSecurity</key>
    <dict>
        <key>NSAllowsArbitraryLoads</key>
        <true/>
    </dict>
</dict>
```

---

## Troubleshooting

### Проблема: CORS ошибка

**Решение:** API уже настроен на прием запросов от любых источников. Если проблема возникает:

```dart
// Убедитесь, что headers корректны
headers: {
  'Content-Type': 'application/json',
  'X-Owner-TG-ID': '$ownerTgId', // для admin endpoints
}
```

### Проблема: Timeout ошибки

**Решение:** Увеличьте timeout:

```dart
final response = await http.get(url).timeout(
  const Duration(seconds: 60),
);
```

### Проблема: X-Owner-TG-ID не работает

**Проверьте:**

1. Telegram ID правильный (получен через @userinfobot)
2. Header добавлен для admin endpoints:
   - POST /api/admin/products
   - PUT /api/admin/products/{id}
   - DELETE /api/admin/products/{id}
   - PUT /api/shops/{token}

```dart
headers: {
  'Content-Type': 'application/json',
  'X-Owner-TG-ID': authProvider.ownerTgId.toString(),
}
```

### Проблема: Изображения не загружаются

**Варианты решения:**

1. Используйте прямые URL изображений (например, с Cloudinary)
2. Добавьте endpoint для upload в backend
3. Используйте cached_network_image для кеширования:

```yaml
dependencies:
  cached_network_image: ^3.3.0
```

```dart
CachedNetworkImage(
  imageUrl: product.imageUrl!,
  placeholder: (context, url) => CircularProgressIndicator(),
  errorWidget: (context, url, error) => Icon(Icons.error),
)
```

### Проблема: SharedPreferences не сохраняет данные

**Решение:**

```dart
// Убедитесь, что await используется
final prefs = await SharedPreferences.getInstance();
await prefs.setInt('owner_tg_id', ownerTgId);

// Проверка сохранения
final saved = prefs.getInt('owner_tg_id');
print('Saved: $saved');
```

### Проблема: Provider не обновляется

**Решение:**

```dart
// Используйте notifyListeners() после изменения данных
void updateData() {
  _data = newData;
  notifyListeners(); // Обязательно!
}

// В UI используйте watch для автообновления
final provider = context.watch<MyProvider>();
```

---

## Best Practices

### 1. Обработка ошибок

```dart
try {
  await apiService.createProduct(...);
  ScaffoldMessenger.of(context).showSnackBar(
    SnackBar(content: Text('Товар создан')),
  );
} on ApiException catch (e) {
  ScaffoldMessenger.of(context).showSnackBar(
    SnackBar(content: Text('Ошибка API: ${e.message}')),
  );
} catch (e) {
  ScaffoldMessenger.of(context).showSnackBar(
    SnackBar(content: Text('Неизвестная ошибка: $e')),
  );
}
```

### 2. Loading States

```dart
// Всегда показывайте индикатор загрузки
if (isLoading) {
  return Center(child: CircularProgressIndicator());
}

// Или в кнопках
ElevatedButton(
  onPressed: isLoading ? null : _handleSubmit,
  child: isLoading
      ? SizedBox(
          width: 20,
          height: 20,
          child: CircularProgressIndicator(strokeWidth: 2),
        )
      : Text('Сохранить'),
)
```

### 3. Form Validation

```dart
// Всегда валидируйте формы
if (!_formKey.currentState!.validate()) {
  return;
}

// Используйте validator
TextFormField(
  validator: (value) {
    if (value == null || value.isEmpty) {
      return 'Поле обязательно';
    }
    if (value.length < 3) {
      return 'Минимум 3 символа';
    }
    return null;
  },
)
```

### 4. Оптимизация изображений

```dart
// Кешируйте изображения
import 'package:cached_network_image/cached_network_image.dart';

CachedNetworkImage(
  imageUrl: imageUrl,
  placeholder: (context, url) => Center(
    child: CircularProgressIndicator(),
  ),
  errorWidget: (context, url, error) => Icon(Icons.error),
  fadeInDuration: Duration(milliseconds: 200),
  memCacheWidth: 400, // Ограничение размера в памяти
)
```

### 5. Pagination (если много товаров)

```dart
// Добавьте пагинацию для больших списков
class ProductsProvider with ChangeNotifier {
  int _currentPage = 1;
  final int _perPage = 20;
  bool _hasMore = true;

  Future<void> loadMore() async {
    if (!_hasMore || _isLoading) return;
    
    _currentPage++;
    final newProducts = await apiService.getProducts(
      shopId,
      page: _currentPage,
      perPage: _perPage,
    );
    
    if (newProducts.length < _perPage) {
      _hasMore = false;
    }
    
    _products.addAll(newProducts);
    notifyListeners();
  }
}
```

---

## Checklist перед запуском

- [ ] Установлен Flutter SDK 3.16.0+
- [ ] Добавлены все dependencies в pubspec.yaml
- [ ] Настроен API_BASE_URL в api_config.dart
- [ ] Получен Telegram User ID через @userinfobot
- [ ] Созданы все models (Shop, Product, Order, OrderItem)
- [ ] Реализован ApiService с retry logic
- [ ] Настроен AuthService для хранения owner_tg_id
- [ ] Созданы Providers (Auth, Shop, Products, Orders)
- [ ] Реализованы все UI screens
- [ ] Настроена навигация в main.dart
- [ ] Добавлены permissions в AndroidManifest.xml и Info.plist
- [ ] Протестировано создание магазина
- [ ] Протестировано добавление товаров
- [ ] Протестирован просмотр заказов
- [ ] Настроен keystore для Android
- [ ] Настроен signing для iOS
- [ ] Собран release build
- [ ] Загружено в Play Store / App Store

---

## Дополнительные ресурсы

### Документация API
- [API Endpoints Reference](./API_ENDPOINTS.md) - полный список endpoints
- Production API: https://shopkit-backend.onrender.com/api

### Flutter ресурсы
- [Flutter Documentation](https://docs.flutter.dev/)
- [Provider Package](https://pub.dev/packages/provider)
- [HTTP Package](https://pub.dev/packages/http)
- [Image Picker](https://pub.dev/packages/image_picker)

### Полезные пакеты
- `cached_network_image` - кеширование изображений
- `intl` - форматирование дат и чисел
- `shared_preferences` - локальное хранилище
- `flutter_launcher_icons` - генерация иконок
- `flutter_native_splash` - splash screen

---

## Заключение

Это полное руководство для создания **admin приложения** на Flutter для владельцев магазинов.

**Ключевые отличия от клиентского приложения:**

| Функция | Admin App (владельцы) | Customer App (TMA) |
|---------|----------------------|-------------------|
| Управление товарами | ✅ Создание, редактирование, удаление | ❌ Только просмотр |
| Корзина | ❌ Не нужна | ✅ Добавление товаров |
| Заказы | ✅ Просмотр всех заказов клиентов | ✅ Создание заказа |
| Аутентификация | X-Owner-TG-ID header | Telegram initData |
| Платформа | Flutter (Android/iOS) | Telegram Mini App (Web) |

**Что дальше?**

1. Скопируйте код из этого гайда
2. Настройте проект под свои нужды
3. Протестируйте на реальном устройстве
4. Опубликуйте в Play Store и App Store

**Готово!** Успехов в разработке! 🚀

---

**Версия документа:** 1.0  
**Дата:** 2026-02-11  
**Production API:** https://shopkit-backend.onrender.com/api

