# ShopKit Frontend API Documentation

## TypeScript Integration Guide

### Base Configuration

```typescript
const API_BASE = 'https://your-production-url.railway.app/api';

interface ShopKitConfig {
  baseURL: string;
  telegramUserId: number;
}

class ShopKitAPI {
  private config: ShopKitConfig;
  
  constructor(config: ShopKitConfig) {
    this.config = config;
  }
  
  private async request<T>(endpoint: string, options?: RequestInit): Promise<T> {
    const response = await fetch(`${this.config.baseURL}${endpoint}`, {
      ...options,
      headers: {
        'Content-Type': 'application/json',
        ...options?.headers,
      },
    });
    
    if (!response.ok) {
      const error = await response.json();
      throw new Error(error.error || 'API Error');
    }
    
    return response.json();
  }
}
```

### TypeScript Interfaces

```typescript
interface Shop {
  id: number;
  shop_token: string;
  title: string;
  description: string;
  currency: string;
  created_at: string;
}

interface Product {
  product_id: string;
  title: string;
  price: number;
  currency: string;
  image_url: string;
  description: string;
  stock_count: number;
  category: string;
}

interface CartItem {
  product_id: string;
  title: string;
  quantity: number;
  price: number;
}

interface Order {
  order_id: string;
  shop_id: number;
  shop_title?: string;
  shop_token?: string;
  total_price: number;
  currency: string;
  status: 'pending' | 'completed' | 'cancelled';
  items: CartItem[];
  created_at: string;
}
```

### API Methods

#### 1. Get Shop
```typescript
async getShop(shopToken: string): Promise<Shop> {
  return this.request<Shop>(`/shops/token/${shopToken}`);
}

// Usage:
const shop = await api.getShop('my_shop_123');
console.log(shop.title);
```

#### 2. Get Products
```typescript
async getProducts(shopId: number): Promise<Product[]> {
  return this.request<Product[]>(`/products/${shopId}`);
}

// Usage:
const products = await api.getProducts(1);
products.forEach(p => console.log(p.title, p.price));
```

#### 3. Save Cart
```typescript
async saveCart(shopId: number, items: CartItem[]): Promise<void> {
  await this.request('/cart', {
    method: 'POST',
    body: JSON.stringify({
      telegram_user_id: this.config.telegramUserId,
      shop_id: shopId,
      items: items,
    }),
  });
}

// Usage:
await api.saveCart(1, [
  { product_id: 'uuid', title: 'Coffee', quantity: 2, price: 250 }
]);
```

#### 4. Get Cart
```typescript
async getCart(shopId: number): Promise<CartItem[]> {
  const response = await this.request<{items: CartItem[]}>(`/cart/${this.config.telegramUserId}?shop_id=${shopId}`);
  return response.items;
}

// Usage:
const cart = await api.getCart(1);
console.log('Cart items:', cart.length);
```

#### 5. Create Order
```typescript
async createOrder(shopId: number, items: CartItem[], customerName: string, customerPhone: string): Promise<Order> {
  const totalPrice = items.reduce((sum, item) => sum + item.price * item.quantity, 0);
  
  return this.request<{order: Order}>('/orders', {
    method: 'POST',
    body: JSON.stringify({
      shop_id: shopId,
      telegram_user_id: this.config.telegramUserId,
      customer_name: customerName,
      customer_phone: customerPhone,
      items: items,
      total_price: totalPrice,
      currency: 'RUB',
    }),
  }).then(res => res.order);
}

// Usage:
const order = await api.createOrder(1, cart, 'John Doe', '+1234567890');
console.log('Order created:', order.order_id);
```

#### 6. Get User Orders (ALL shops)
```typescript
async getUserOrders(): Promise<Order[]> {
  return this.request<Order[]>(`/user/orders/${this.config.telegramUserId}`);
}

// Usage:
const orders = await api.getUserOrders();
orders.forEach(order => {
  console.log(`Shop: ${order.shop_title}, Total: ${order.total_price}`);
});
```

### Complete Example

```typescript
// Initialize API
const userId = window.Telegram.WebApp.initDataUnsafe.user?.id || 0;
const api = new ShopKitAPI({
  baseURL: 'https://shopkit-production.railway.app/api',
  telegramUserId: userId,
});

// Get shop token from URL
const shopToken = window.Telegram.WebApp.initDataUnsafe.start_param || 'default';

// Load shop and products
const shop = await api.getShop(shopToken);
const products = await api.getProducts(shop.id);

// Load saved cart
let cart = await api.getCart(shop.id);

// Add to cart
function addToCart(product: Product) {
  const existing = cart.find(item => item.product_id === product.product_id);
  
  if (existing) {
    existing.quantity++;
  } else {
    cart.push({
      product_id: product.product_id,
      title: product.title,
      quantity: 1,
      price: product.price,
    });
  }
  
  // Save to server
  api.saveCart(shop.id, cart);
}

// Checkout
async function checkout() {
  const user = window.Telegram.WebApp.initDataUnsafe.user;
  
  const order = await api.createOrder(
    shop.id,
    cart,
    user.first_name + ' ' + (user.last_name || ''),
    '' // phone will be requested
  );
  
  // Clear cart
  cart = [];
  api.saveCart(shop.id, []);
  
  alert(`Order created: ${order.order_id}`);
}
```

### Error Handling

```typescript
try {
  const shop = await api.getShop('invalid_token');
} catch (error) {
  if (error.message === 'Shop not found') {
    // Handle not found
  } else {
    // Handle other errors
  }
}
```

## API Endpoints Reference

### Shop Endpoints
- `GET /api/shops/token/{shop_token}` - Get shop by token
- `POST /api/shops` - Create new shop
- `PUT /api/shops/{shop_token}` - Update shop

### Product Endpoints
- `GET /api/products/{shop_id}` - Get all products for a shop
- `GET /api/products/detail/{product_id}` - Get product details
- `POST /api/admin/products` - Create product (admin)
- `PUT /api/admin/products/{product_id}` - Update product (admin)
- `DELETE /api/admin/products/{product_id}` - Delete product (admin)

### Cart Endpoints
- `POST /api/cart` - Save cart
- `GET /api/cart/{telegram_user_id}?shop_id={shop_id}` - Get cart
- `DELETE /api/cart/{telegram_user_id}?shop_id={shop_id}` - Clear cart

### Order Endpoints
- `POST /api/orders` - Create order
- `GET /api/user/orders/{telegram_user_id}` - Get all user orders

## Testing with cURL

```bash
# Get shop
curl http://localhost:8080/api/shops/token/my_shop_123

# Get products
curl http://localhost:8080/api/products/1

# Save cart
curl -X POST http://localhost:8080/api/cart \
  -H "Content-Type: application/json" \
  -d '{
    "telegram_user_id": 123456789,
    "shop_id": 1,
    "items": [
      {"product_id":"uuid","title":"Coffee","quantity":2,"price":250}
    ]
  }'

# Get cart
curl "http://localhost:8080/api/cart/123456789?shop_id=1"

# Create order
curl -X POST http://localhost:8080/api/orders \
  -H "Content-Type: application/json" \
  -d '{
    "shop_id": 1,
    "telegram_user_id": 123456789,
    "customer_name": "John Doe",
    "customer_phone": "+1234567890",
    "items": [{"product_id":"uuid","title":"Coffee","quantity":2,"price":250}],
    "total_price": 500,
    "currency": "RUB"
  }'

# Get user orders
curl http://localhost:8080/api/user/orders/123456789
```
