# 📚 Complete Frontend Integration Guide for Telegram Mini Apps

> **Production API:** `https://shopkit-backend.onrender.com/api`  
> **Status:** ✅ Production-ready

## Table of Contents

1. [Quick Start](#quick-start)
2. [Production Configuration](#production-configuration)
3. [TypeScript Type Definitions](#typescript-type-definitions)
4. [Complete API Client](#complete-api-client)
5. [React Hooks](#react-hooks)
6. [Page Components](#page-components)
7. [Telegram Mini App Integration](#telegram-mini-app-integration)
8. [Styling Guide](#styling-guide)
9. [Testing Guide](#testing-guide)
10. [Production Examples](#production-examples)
11. [Deployment Checklist](#deployment-checklist)
12. [Troubleshooting](#troubleshooting)

---

## Quick Start

### Prerequisites

```bash
npm install @twa-dev/sdk
```

### Basic Setup

```typescript
import WebApp from '@twa-dev/sdk'

// Initialize Telegram WebApp
WebApp.ready()

// Get user info
const userId = WebApp.initDataUnsafe.user?.id || 0
const userName = WebApp.initDataUnsafe.user?.first_name || 'Guest'

// Get shop token from deep link
const shopToken = WebApp.initDataUnsafe.start_param || 'demo'

console.log(`User ${userName} (${userId}) opened shop: ${shopToken}`)
```

---

## Production Configuration

### Environment Setup

Create a `.env` file:

```env
VITE_API_BASE_URL=https://shopkit-backend.onrender.com/api
```

### TypeScript Config

`tsconfig.json`:
```json
{
  "compilerOptions": {
    "target": "ES2020",
    "lib": ["ES2020", "DOM", "DOM.Iterable"],
    "module": "ESNext",
    "skipLibCheck": true,
    "moduleResolution": "bundler",
    "resolveJsonModule": true,
    "isolatedModules": true,
    "noEmit": true,
    "jsx": "react-jsx",
    "strict": true,
    "noUnusedLocals": true,
    "noUnusedParameters": true,
    "noFallthroughCasesInSwitch": true
  },
  "include": ["src"]
}
```

### Telegram WebApp Script

Add to `index.html`:
```html
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>ShopKit Mini App</title>
    <script src="https://telegram.org/js/telegram-web-app.js"></script>
  </head>
  <body>
    <div id="root"></div>
    <script type="module" src="/src/main.tsx"></script>
  </body>
</html>
```

---

## TypeScript Type Definitions

Based on the database schema in `sql/schema.sql`:

### Core Types

```typescript
// API Configuration
interface APIConfig {
  baseURL: string;
  timeout?: number;
  retries?: number;
}

// Shop Type
interface Shop {
  id: number;
  shop_token: string;
  owner_tg_id: number;
  title: string;
  description: string;
  currency: string;
  created_at: string;
  updated_at: string;
  products_count?: number;
}

// Product Type
interface Product {
  id: number;
  product_id: string; // UUID
  shop_id: number;
  title: string;
  price: number;
  currency: string;
  image_url?: string;
  description?: string;
  stock_count: number;
  is_active: boolean;
  category?: string;
  tags?: string[];
  created_at: string;
  updated_at: string;
}

// Product Detail (includes shop info)
interface ProductDetail extends Product {
  shop: {
    id: number;
    title: string;
    shop_token: string;
  };
}

// Cart Item Type
interface CartItem {
  product_id: string;
  title: string;
  quantity: number;
  price: number;
}

// Cart Response
interface CartResponse {
  telegram_user_id: number;
  shop_id: number;
  items: CartItem[];
  updated_at?: string;
}

// Order Type
interface Order {
  id: number;
  order_id: string; // UUID
  shop_id: number;
  shop_title?: string; // Populated in /user/orders endpoint
  shop_token?: string; // Populated in /user/orders endpoint
  telegram_user_id: number;
  customer_name: string;
  customer_phone?: string;
  items: CartItem[];
  total_price: number;
  currency: string;
  status: 'pending' | 'completed' | 'cancelled';
  payment_method?: string;
  notes?: string;
  created_at: string;
  updated_at: string;
}

// Order Creation Request
interface CreateOrderRequest {
  shop_id: number;
  telegram_user_id: number;
  customer_name: string;
  customer_phone?: string;
  items: CartItem[];
  total_price: number;
  currency: string;
  payment_method?: string;
  notes?: string;
}

// API Error
class APIError extends Error {
  constructor(
    message: string,
    public statusCode?: number,
    public code?: string,
    public details?: unknown
  ) {
    super(message);
    this.name = 'APIError';
  }
}
```

---

## Complete API Client

Full implementation with error handling, retry logic, and TypeScript generics:

```typescript
// src/lib/api.ts

const API_BASE_URL = import.meta.env.VITE_API_BASE_URL || 'https://shopkit-backend.onrender.com/api';
const DEFAULT_TIMEOUT = 10000; // 10 seconds
const DEFAULT_RETRIES = 3;

class ShopKitAPI {
  private baseURL: string;
  private timeout: number;
  private retries: number;

  constructor(config?: Partial<APIConfig>) {
    this.baseURL = config?.baseURL || API_BASE_URL;
    this.timeout = config?.timeout || DEFAULT_TIMEOUT;
    this.retries = config?.retries || DEFAULT_RETRIES;
  }

  private async request<T>(
    endpoint: string,
    options?: RequestInit,
    attempt = 1
  ): Promise<T> {
    const controller = new AbortController();
    const timeoutId = setTimeout(() => controller.abort(), this.timeout);

    try {
      const response = await fetch(`${this.baseURL}${endpoint}`, {
        ...options,
        headers: {
          'Content-Type': 'application/json',
          ...options?.headers,
        },
        signal: controller.signal,
      });

      clearTimeout(timeoutId);

      if (!response.ok) {
        const errorData = await response.json().catch(() => ({}));
        throw new APIError(
          errorData.error || `HTTP ${response.status}`,
          response.status,
          errorData.code,
          errorData.details
        );
      }

      return response.json();
    } catch (error) {
      clearTimeout(timeoutId);

      // Retry on network errors or 5xx errors
      if (
        attempt < this.retries &&
        (error instanceof TypeError || // Network error
          (error instanceof APIError && error.statusCode && error.statusCode >= 500))
      ) {
        // Exponential backoff: 1s, 2s, 4s
        const delay = Math.pow(2, attempt - 1) * 1000;
        await new Promise(resolve => setTimeout(resolve, delay));
        return this.request<T>(endpoint, options, attempt + 1);
      }

      if (error instanceof APIError) {
        throw error;
      }

      throw new APIError('Network error', undefined, 'NETWORK_ERROR', error);
    }
  }

  // ============================================================================
  // SHOP ENDPOINTS
  // ============================================================================

  /**
   * Get shop by token (from Telegram start_param)
   * @param shopToken - Shop token from deep link
   */
  async getShop(shopToken: string): Promise<Shop> {
    return this.request<Shop>(`/shops/token/${shopToken}`);
  }

  /**
   * Create new shop (admin)
   * @param data - Shop creation data
   */
  async createShop(data: {
    owner_tg_id: number;
    title: string;
    description?: string;
    currency?: string;
    shop_token?: string;
  }): Promise<{ success: boolean; shop: Shop; message: string }> {
    return this.request('/shops', {
      method: 'POST',
      body: JSON.stringify(data),
    });
  }

  /**
   * Update shop (admin, requires X-Owner-TG-ID header)
   * @param shopToken - Shop token
   * @param ownerTgId - Owner Telegram ID for authentication
   * @param data - Fields to update
   */
  async updateShop(
    shopToken: string,
    ownerTgId: number,
    data: {
      title?: string;
      description?: string;
      currency?: string;
    }
  ): Promise<{ success: boolean; shop_token: string; title: string; currency: string; updated_at: string }> {
    return this.request(`/shops/${shopToken}`, {
      method: 'PUT',
      headers: {
        'X-Owner-TG-ID': ownerTgId.toString(),
      },
      body: JSON.stringify(data),
    });
  }

  // ============================================================================
  // PRODUCT ENDPOINTS
  // ============================================================================

  /**
   * Get all active products for a shop
   * @param shopId - Shop ID
   */
  async getProducts(shopId: number): Promise<Product[]> {
    return this.request<Product[]>(`/products/${shopId}`);
  }

  /**
   * Get single product details
   * @param productId - Product UUID
   */
  async getProductDetail(productId: string): Promise<ProductDetail> {
    return this.request<ProductDetail>(`/products/detail/${productId}`);
  }

  /**
   * Create product (admin, requires X-Owner-TG-ID header)
   * @param ownerTgId - Owner Telegram ID for authentication
   * @param data - Product creation data
   */
  async createProduct(
    ownerTgId: number,
    data: {
      shop_id: number;
      title: string;
      price: number;
      currency?: string;
      image_url?: string;
      description?: string;
      stock_count?: number;
      category?: string;
    }
  ): Promise<{ success: boolean; product: Product }> {
    return this.request('/admin/products', {
      method: 'POST',
      headers: {
        'X-Owner-TG-ID': ownerTgId.toString(),
      },
      body: JSON.stringify(data),
    });
  }

  /**
   * Update product (admin, requires X-Owner-TG-ID header)
   * @param productId - Product UUID
   * @param ownerTgId - Owner Telegram ID for authentication
   * @param data - Fields to update
   */
  async updateProduct(
    productId: string,
    ownerTgId: number,
    data: {
      title?: string;
      price?: number;
      description?: string;
      image_url?: string;
      stock_count?: number;
      category?: string;
    }
  ): Promise<{ success: boolean; product_id: string; title: string; price: number; updated_at: string }> {
    return this.request(`/admin/products/${productId}`, {
      method: 'PUT',
      headers: {
        'X-Owner-TG-ID': ownerTgId.toString(),
      },
      body: JSON.stringify(data),
    });
  }

  /**
   * Delete product (admin, requires X-Owner-TG-ID header)
   * @param productId - Product UUID
   * @param ownerTgId - Owner Telegram ID for authentication
   */
  async deleteProduct(
    productId: string,
    ownerTgId: number
  ): Promise<{ success: boolean; product_id: string; message: string }> {
    return this.request(`/admin/products/${productId}`, {
      method: 'DELETE',
      headers: {
        'X-Owner-TG-ID': ownerTgId.toString(),
      },
    });
  }

  // ============================================================================
  // CART ENDPOINTS
  // ============================================================================

  /**
   * Save/update cart (upsert operation)
   * @param userId - Telegram user ID
   * @param shopId - Shop ID
   * @param items - Cart items
   */
  async saveCart(
    userId: number,
    shopId: number,
    items: CartItem[]
  ): Promise<{ success: boolean; updated_at: string }> {
    return this.request('/cart', {
      method: 'POST',
      body: JSON.stringify({
        telegram_user_id: userId,
        shop_id: shopId,
        items,
      }),
    });
  }

  /**
   * Get cart for user and shop
   * @param userId - Telegram user ID
   * @param shopId - Shop ID
   */
  async getCart(userId: number, shopId: number): Promise<CartResponse> {
    return this.request<CartResponse>(`/cart/${userId}?shop_id=${shopId}`);
  }

  /**
   * Clear cart for user and shop
   * @param userId - Telegram user ID
   * @param shopId - Shop ID
   */
  async clearCart(userId: number, shopId: number): Promise<{ success: boolean }> {
    return this.request(`/cart/${userId}?shop_id=${shopId}`, {
      method: 'DELETE',
    });
  }

  // ============================================================================
  // ORDER ENDPOINTS
  // ============================================================================

  /**
   * Create new order
   * @param data - Order creation data
   */
  async createOrder(data: CreateOrderRequest): Promise<{ success: boolean; order: Order }> {
    return this.request('/orders', {
      method: 'POST',
      body: JSON.stringify(data),
    });
  }

  /**
   * Get ALL user orders from ALL shops (critical feature!)
   * Returns orders from every shop the user has ordered from
   * @param userId - Telegram user ID
   */
  async getUserOrders(userId: number): Promise<Order[]> {
    return this.request<Order[]>(`/user/orders/${userId}`);
  }
}

// Export singleton instance
export const api = new ShopKitAPI();
export { ShopKitAPI, APIError };
export type { Shop, Product, ProductDetail, CartItem, CartResponse, Order, CreateOrderRequest };
```

---

## React Hooks

Production-ready React hooks with optimistic updates and error handling:

### 1. useShop Hook

```typescript
// src/hooks/useShop.ts

import { useState, useEffect } from 'react';
import { api, Shop, APIError } from '../lib/api';

export function useShop(shopToken: string) {
  const [shop, setShop] = useState<Shop | null>(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  const load = async () => {
    try {
      setLoading(true);
      setError(null);
      const data = await api.getShop(shopToken);
      setShop(data);
    } catch (err) {
      if (err instanceof APIError) {
        setError(err.message);
      } else {
        setError('Failed to load shop');
      }
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    if (shopToken) {
      load();
    }
  }, [shopToken]);

  return { shop, loading, error, refetch: load };
}
```

### 2. useProducts Hook

```typescript
// src/hooks/useProducts.ts

import { useState, useEffect, useMemo } from 'react';
import { api, Product, APIError } from '../lib/api';

export function useProducts(shopId: number | null) {
  const [products, setProducts] = useState<Product[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  const load = async () => {
    if (!shopId) return;

    try {
      setLoading(true);
      setError(null);
      const data = await api.getProducts(shopId);
      setProducts(data);
    } catch (err) {
      if (err instanceof APIError) {
        setError(err.message);
      } else {
        setError('Failed to load products');
      }
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    load();
  }, [shopId]);

  // Filter by category
  const getProductsByCategory = (category: string) => {
    return products.filter(p => p.category === category);
  };

  // Get unique categories
  const categories = useMemo(() => {
    const cats = products
      .map(p => p.category)
      .filter((c): c is string => !!c);
    return Array.from(new Set(cats));
  }, [products]);

  // Check stock
  const isInStock = (productId: string) => {
    const product = products.find(p => p.product_id === productId);
    return product ? product.stock_count > 0 : false;
  };

  return {
    products,
    loading,
    error,
    refetch: load,
    getProductsByCategory,
    categories,
    isInStock,
  };
}
```

### 3. useCart Hook

```typescript
// src/hooks/useCart.ts

import { useState, useEffect, useCallback, useRef } from 'react';
import { api, CartItem, APIError } from '../lib/api';

const DEBOUNCE_DELAY = 500; // 500ms debounce for auto-save

export function useCart(userId: number, shopId: number | null) {
  const [items, setItems] = useState<CartItem[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [saving, setSaving] = useState(false);
  
  const saveTimeoutRef = useRef<NodeJS.Timeout>();

  // Load cart from server
  const load = useCallback(async () => {
    if (!shopId) return;

    try {
      setLoading(true);
      setError(null);
      const data = await api.getCart(userId, shopId);
      setItems(data.items || []);
    } catch (err) {
      if (err instanceof APIError && err.statusCode === 404) {
        // No cart yet, that's ok
        setItems([]);
      } else if (err instanceof APIError) {
        setError(err.message);
      } else {
        setError('Failed to load cart');
      }
    } finally {
      setLoading(false);
    }
  }, [userId, shopId]);

  // Save cart to server (debounced)
  const save = useCallback(
    async (newItems: CartItem[]) => {
      if (!shopId) return;

      // Clear existing timeout
      if (saveTimeoutRef.current) {
        clearTimeout(saveTimeoutRef.current);
      }

      // Debounce the save
      saveTimeoutRef.current = setTimeout(async () => {
        try {
          setSaving(true);
          await api.saveCart(userId, shopId, newItems);
        } catch (err) {
          console.error('Failed to save cart:', err);
          // Don't show error to user, just log it
        } finally {
          setSaving(false);
        }
      }, DEBOUNCE_DELAY);
    },
    [userId, shopId]
  );

  // Add item to cart
  const addItem = useCallback(
    (product: { product_id: string; title: string; price: number }, quantity = 1) => {
      setItems(currentItems => {
        const existing = currentItems.find(item => item.product_id === product.product_id);

        let newItems: CartItem[];
        if (existing) {
          // Update quantity
          newItems = currentItems.map(item =>
            item.product_id === product.product_id
              ? { ...item, quantity: item.quantity + quantity }
              : item
          );
        } else {
          // Add new item
          newItems = [
            ...currentItems,
            {
              product_id: product.product_id,
              title: product.title,
              price: product.price,
              quantity,
            },
          ];
        }

        save(newItems);
        return newItems;
      });
    },
    [save]
  );

  // Remove item from cart
  const removeItem = useCallback(
    (productId: string) => {
      setItems(currentItems => {
        const newItems = currentItems.filter(item => item.product_id !== productId);
        save(newItems);
        return newItems;
      });
    },
    [save]
  );

  // Update item quantity
  const updateQuantity = useCallback(
    (productId: string, quantity: number) => {
      if (quantity <= 0) {
        removeItem(productId);
        return;
      }

      setItems(currentItems => {
        const newItems = currentItems.map(item =>
          item.product_id === productId ? { ...item, quantity } : item
        );
        save(newItems);
        return newItems;
      });
    },
    [save, removeItem]
  );

  // Clear cart
  const clear = useCallback(async () => {
    if (!shopId) return;

    try {
      await api.clearCart(userId, shopId);
      setItems([]);
    } catch (err) {
      console.error('Failed to clear cart:', err);
    }
  }, [userId, shopId]);

  // Calculate totals
  const total = items.reduce((sum, item) => sum + item.price * item.quantity, 0);
  const itemCount = items.reduce((sum, item) => sum + item.quantity, 0);

  // Load on mount
  useEffect(() => {
    load();
  }, [load]);

  // Cleanup timeout on unmount
  useEffect(() => {
    return () => {
      if (saveTimeoutRef.current) {
        clearTimeout(saveTimeoutRef.current);
      }
    };
  }, []);

  return {
    items,
    loading,
    error,
    saving,
    total,
    itemCount,
    addItem,
    removeItem,
    updateQuantity,
    clear,
    refetch: load,
  };
}
```

### 4. useOrders Hook

```typescript
// src/hooks/useOrders.ts

import { useState, useEffect, useCallback, useMemo } from 'react';
import { api, Order, APIError } from '../lib/api';

export function useOrders(userId: number) {
  const [orders, setOrders] = useState<Order[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  const load = useCallback(async () => {
    try {
      setLoading(true);
      setError(null);
      const data = await api.getUserOrders(userId);
      setOrders(data);
    } catch (err) {
      if (err instanceof APIError) {
        setError(err.message);
      } else {
        setError('Failed to load orders');
      }
    } finally {
      setLoading(false);
    }
  }, [userId]);

  useEffect(() => {
    if (userId) {
      load();
    }
  }, [userId, load]);

  // Filter by status
  const getOrdersByStatus = useCallback(
    (status: Order['status']) => {
      return orders.filter(o => o.status === status);
    },
    [orders]
  );

  // Get orders by shop
  const getOrdersByShop = useCallback(
    (shopToken: string) => {
      return orders.filter(o => o.shop_token === shopToken);
    },
    [orders]
  );

  // Memoized stats
  const stats = useMemo(() => {
    return {
      total: orders.length,
      pending: orders.filter(o => o.status === 'pending').length,
      completed: orders.filter(o => o.status === 'completed').length,
      cancelled: orders.filter(o => o.status === 'cancelled').length,
      totalSpent: orders
        .filter(o => o.status !== 'cancelled')
        .reduce((sum, o) => sum + o.total_price, 0),
    };
  }, [orders]);

  return {
    orders,
    loading,
    error,
    refetch: load,
    getOrdersByStatus,
    getOrdersByShop,
    stats,
  };
}
```

---

## Page Components

Complete, production-ready React components with TypeScript:

### 1. ShopPage Component

```typescript
// src/pages/ShopPage.tsx

import { useEffect, useState } from 'react';
import WebApp from '@twa-dev/sdk';
import { useShop } from '../hooks/useShop';
import { useProducts } from '../hooks/useProducts';
import { useCart } from '../hooks/useCart';

export function ShopPage() {
  const shopToken = WebApp.initDataUnsafe.start_param || 'demo';
  const userId = WebApp.initDataUnsafe.user?.id || 0;

  const { shop, loading: shopLoading, error: shopError } = useShop(shopToken);
  const { products, loading: productsLoading, categories } = useProducts(shop?.id || null);
  const { addItem, itemCount } = useCart(userId, shop?.id || null);

  const [selectedCategory, setSelectedCategory] = useState<string | null>(null);

  useEffect(() => {
    WebApp.ready();
    WebApp.expand();
  }, []);

  if (shopLoading) {
    return (
      <div className="loading-container">
        <div className="spinner"></div>
        <p>Loading shop...</p>
      </div>
    );
  }

  if (shopError || !shop) {
    return (
      <div className="error-container">
        <h2>Shop Not Found</h2>
        <p>{shopError || 'This shop does not exist'}</p>
      </div>
    );
  }

  const filteredProducts = selectedCategory
    ? products.filter(p => p.category === selectedCategory)
    : products;

  const handleAddToCart = (product: typeof products[0]) => {
    addItem(product);
    WebApp.showAlert(`${product.title} added to cart!`);
  };

  return (
    <div className="shop-page">
      {/* Header */}
      <header className="shop-header">
        <h1>{shop.title}</h1>
        {shop.description && <p className="shop-description">{shop.description}</p>}
        
        {itemCount > 0 && (
          <div className="cart-badge">
            <button onClick={() => (window.location.href = '/cart')}>
              🛒 Cart ({itemCount})
            </button>
          </div>
        )}
      </header>

      {/* Categories */}
      {categories.length > 0 && (
        <div className="categories">
          <button
            className={!selectedCategory ? 'active' : ''}
            onClick={() => setSelectedCategory(null)}
          >
            All
          </button>
          {categories.map(cat => (
            <button
              key={cat}
              className={selectedCategory === cat ? 'active' : ''}
              onClick={() => setSelectedCategory(cat)}
            >
              {cat}
            </button>
          ))}
        </div>
      )}

      {/* Products Grid */}
      {productsLoading ? (
        <div className="loading">Loading products...</div>
      ) : filteredProducts.length === 0 ? (
        <div className="empty-state">
          <p>No products available</p>
        </div>
      ) : (
        <div className="products-grid">
          {filteredProducts.map(product => (
            <div key={product.product_id} className="product-card">
              {product.image_url && (
                <img src={product.image_url} alt={product.title} />
              )}
              <h3>{product.title}</h3>
              {product.description && (
                <p className="product-description">{product.description}</p>
              )}
              <div className="product-footer">
                <span className="price">
                  {product.price} {product.currency}
                </span>
                {product.stock_count > 0 ? (
                  <button onClick={() => handleAddToCart(product)}>
                    Add to Cart
                  </button>
                ) : (
                  <button disabled>Out of Stock</button>
                )}
              </div>
            </div>
          ))}
        </div>
      )}
    </div>
  );
}
```

### 2. CartPage Component

```typescript
// src/pages/CartPage.tsx

import { useEffect } from 'react';
import WebApp from '@twa-dev/sdk';
import { useCart } from '../hooks/useCart';
import { useShop } from '../hooks/useShop';

export function CartPage() {
  const shopToken = WebApp.initDataUnsafe.start_param || 'demo';
  const userId = WebApp.initDataUnsafe.user?.id || 0;

  const { shop } = useShop(shopToken);
  const {
    items,
    loading,
    total,
    updateQuantity,
    removeItem,
    saving,
  } = useCart(userId, shop?.id || null);

  useEffect(() => {
    WebApp.ready();
    WebApp.BackButton.show();
    WebApp.BackButton.onClick(() => {
      window.history.back();
    });

    return () => {
      WebApp.BackButton.hide();
    };
  }, []);

  const handleCheckout = () => {
    window.location.href = '/checkout';
  };

  if (loading) {
    return <div className="loading">Loading cart...</div>;
  }

  if (items.length === 0) {
    return (
      <div className="empty-cart">
        <h2>Your cart is empty</h2>
        <button onClick={() => (window.location.href = '/')}>
          Continue Shopping
        </button>
      </div>
    );
  }

  return (
    <div className="cart-page">
      <h1>Shopping Cart</h1>

      {saving && <div className="saving-indicator">Saving...</div>}

      <div className="cart-items">
        {items.map(item => (
          <div key={item.product_id} className="cart-item">
            <div className="item-info">
              <h3>{item.title}</h3>
              <p className="item-price">
                {item.price} {shop?.currency || 'RUB'} × {item.quantity}
              </p>
            </div>

            <div className="item-controls">
              <div className="quantity-controls">
                <button
                  onClick={() => updateQuantity(item.product_id, item.quantity - 1)}
                >
                  −
                </button>
                <span>{item.quantity}</span>
                <button
                  onClick={() => updateQuantity(item.product_id, item.quantity + 1)}
                >
                  +
                </button>
              </div>

              <button
                className="remove-btn"
                onClick={() => removeItem(item.product_id)}
              >
                Remove
              </button>
            </div>

            <div className="item-total">
              {item.price * item.quantity} {shop?.currency || 'RUB'}
            </div>
          </div>
        ))}
      </div>

      <div className="cart-footer">
        <div className="cart-total">
          <span>Total:</span>
          <span className="total-amount">
            {total} {shop?.currency || 'RUB'}
          </span>
        </div>

        <button className="checkout-btn" onClick={handleCheckout}>
          Proceed to Checkout
        </button>
      </div>
    </div>
  );
}
```

### 3. CheckoutPage Component

```typescript
// src/pages/CheckoutPage.tsx

import { useState, useEffect } from 'react';
import WebApp from '@twa-dev/sdk';
import { useCart } from '../hooks/useCart';
import { useShop } from '../hooks/useShop';
import { api } from '../lib/api';

export function CheckoutPage() {
  const shopToken = WebApp.initDataUnsafe.start_param || 'demo';
  const userId = WebApp.initDataUnsafe.user?.id || 0;
  const user = WebApp.initDataUnsafe.user;

  const { shop } = useShop(shopToken);
  const { items, total, clear } = useCart(userId, shop?.id || null);

  const [customerName, setCustomerName] = useState(
    user ? `${user.first_name} ${user.last_name || ''}`.trim() : ''
  );
  const [customerPhone, setCustomerPhone] = useState('');
  const [notes, setNotes] = useState('');
  const [submitting, setSubmitting] = useState(false);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    WebApp.ready();
    WebApp.BackButton.show();
    WebApp.BackButton.onClick(() => window.history.back());

    // Setup MainButton for checkout
    WebApp.MainButton.setText(`Pay ${total} ${shop?.currency || 'RUB'}`);
    WebApp.MainButton.show();
    WebApp.MainButton.onClick(handleSubmit);

    return () => {
      WebApp.BackButton.hide();
      WebApp.MainButton.hide();
      WebApp.MainButton.offClick(handleSubmit);
    };
  }, [total, shop]);

  const handleSubmit = async () => {
    if (!shop) return;

    // Validation
    if (!customerName.trim()) {
      WebApp.showAlert('Please enter your name');
      return;
    }

    try {
      setSubmitting(true);
      setError(null);
      WebApp.MainButton.showProgress();

      const { order } = await api.createOrder({
        shop_id: shop.id,
        telegram_user_id: userId,
        customer_name: customerName,
        customer_phone: customerPhone || undefined,
        items,
        total_price: total,
        currency: shop.currency,
        notes: notes || undefined,
      });

      // Clear cart
      await clear();

      WebApp.MainButton.hideProgress();
      WebApp.showAlert(
        `Order created successfully! Order ID: ${order.order_id.slice(0, 8)}...`,
        () => {
          window.location.href = '/orders';
        }
      );
    } catch (err) {
      WebApp.MainButton.hideProgress();
      setError(err instanceof Error ? err.message : 'Failed to create order');
      WebApp.showAlert('Failed to create order. Please try again.');
    } finally {
      setSubmitting(false);
    }
  };

  if (items.length === 0) {
    return (
      <div className="empty-state">
        <p>Your cart is empty</p>
        <button onClick={() => (window.location.href = '/')}>
          Go Shopping
        </button>
      </div>
    );
  }

  return (
    <div className="checkout-page">
      <h1>Checkout</h1>

      {error && <div className="error-message">{error}</div>}

      <div className="order-summary">
        <h2>Order Summary</h2>
        {items.map(item => (
          <div key={item.product_id} className="summary-item">
            <span>
              {item.title} × {item.quantity}
            </span>
            <span>{item.price * item.quantity} {shop?.currency}</span>
          </div>
        ))}
        <div className="summary-total">
          <strong>Total:</strong>
          <strong>{total} {shop?.currency}</strong>
        </div>
      </div>

      <form className="checkout-form" onSubmit={(e) => e.preventDefault()}>
        <div className="form-group">
          <label htmlFor="name">Full Name *</label>
          <input
            id="name"
            type="text"
            value={customerName}
            onChange={(e) => setCustomerName(e.target.value)}
            placeholder="John Doe"
            required
          />
        </div>

        <div className="form-group">
          <label htmlFor="phone">Phone Number (optional)</label>
          <input
            id="phone"
            type="tel"
            value={customerPhone}
            onChange={(e) => setCustomerPhone(e.target.value)}
            placeholder="+1234567890"
          />
        </div>

        <div className="form-group">
          <label htmlFor="notes">Order Notes (optional)</label>
          <textarea
            id="notes"
            value={notes}
            onChange={(e) => setNotes(e.target.value)}
            placeholder="Any special requests?"
            rows={3}
          />
        </div>
      </form>

      <p className="checkout-hint">
        Click the button below to complete your order
      </p>
    </div>
  );
}
```

### 4. OrdersPage Component

```typescript
// src/pages/OrdersPage.tsx

import { useEffect } from 'react';
import WebApp from '@twa-dev/sdk';
import { useOrders } from '../hooks/useOrders';

export function OrdersPage() {
  const userId = WebApp.initDataUnsafe.user?.id || 0;
  const { orders, loading, stats } = useOrders(userId);

  useEffect(() => {
    WebApp.ready();
  }, []);

  if (loading) {
    return <div className="loading">Loading orders...</div>;
  }

  if (orders.length === 0) {
    return (
      <div className="empty-state">
        <h2>No orders yet</h2>
        <p>Your order history from all shops will appear here</p>
      </div>
    );
  }

  return (
    <div className="orders-page">
      <h1>My Orders</h1>

      <div className="order-stats">
        <div className="stat">
          <span className="stat-value">{stats.total}</span>
          <span className="stat-label">Total Orders</span>
        </div>
        <div className="stat">
          <span className="stat-value">{stats.pending}</span>
          <span className="stat-label">Pending</span>
        </div>
        <div className="stat">
          <span className="stat-value">{stats.completed}</span>
          <span className="stat-label">Completed</span>
        </div>
      </div>

      <div className="orders-list">
        {orders.map(order => (
          <div
            key={order.order_id}
            className="order-card"
            onClick={() => (window.location.href = `/orders/${order.order_id}`)}
          >
            <div className="order-header">
              <div>
                <h3>{order.shop_title}</h3>
                <p className="order-id">#{order.order_id.slice(0, 8)}</p>
              </div>
              <span className={`status status-${order.status}`}>
                {order.status}
              </span>
            </div>

            <div className="order-items">
              {order.items.slice(0, 3).map((item, i) => (
                <p key={i}>
                  {item.title} × {item.quantity}
                </p>
              ))}
              {order.items.length > 3 && (
                <p>+ {order.items.length - 3} more items</p>
              )}
            </div>

            <div className="order-footer">
              <span className="order-total">
                {order.total_price} {order.currency}
              </span>
              <span className="order-date">
                {new Date(order.created_at).toLocaleDateString()}
              </span>
            </div>
          </div>
        ))}
      </div>
    </div>
  );
}
```

### 5. OrderDetailPage Component

```typescript
// src/pages/OrderDetailPage.tsx

import { useState, useEffect } from 'react';
import WebApp from '@twa-dev/sdk';
import { useOrders } from '../hooks/useOrders';
import { useParams } from 'react-router-dom';

export function OrderDetailPage() {
  const { orderId } = useParams<{ orderId: string }>();
  const userId = WebApp.initDataUnsafe.user?.id || 0;
  const { orders, loading } = useOrders(userId);

  const order = orders.find(o => o.order_id === orderId);

  useEffect(() => {
    WebApp.ready();
    WebApp.BackButton.show();
    WebApp.BackButton.onClick(() => window.history.back());

    return () => {
      WebApp.BackButton.hide();
    };
  }, []);

  if (loading) {
    return <div className="loading">Loading order...</div>;
  }

  if (!order) {
    return (
      <div className="error-container">
        <h2>Order Not Found</h2>
        <button onClick={() => window.history.back()}>Go Back</button>
      </div>
    );
  }

  return (
    <div className="order-detail-page">
      <div className="order-header">
        <h1>Order #{order.order_id.slice(0, 8)}</h1>
        <span className={`status status-${order.status}`}>
          {order.status}
        </span>
      </div>

      <div className="order-info">
        <div className="info-row">
          <span>Shop:</span>
          <span>{order.shop_title}</span>
        </div>
        <div className="info-row">
          <span>Date:</span>
          <span>{new Date(order.created_at).toLocaleString()}</span>
        </div>
        <div className="info-row">
          <span>Customer:</span>
          <span>{order.customer_name}</span>
        </div>
        {order.customer_phone && (
          <div className="info-row">
            <span>Phone:</span>
            <span>{order.customer_phone}</span>
          </div>
        )}
        {order.notes && (
          <div className="info-row">
            <span>Notes:</span>
            <span>{order.notes}</span>
          </div>
        )}
      </div>

      <div className="order-items">
        <h2>Items</h2>
        {order.items.map((item, index) => (
          <div key={index} className="order-item">
            <div className="item-info">
              <h3>{item.title}</h3>
              <p>Quantity: {item.quantity}</p>
            </div>
            <div className="item-price">
              {item.price * item.quantity} {order.currency}
            </div>
          </div>
        ))}
      </div>

      <div className="order-total">
        <span>Total:</span>
        <span className="total-amount">
          {order.total_price} {order.currency}
        </span>
      </div>
    </div>
  );
}
```

---

## Telegram Mini App Integration

### Getting User Information

```typescript
import WebApp from '@twa-dev/sdk';

// Initialize on app start
WebApp.ready();
WebApp.expand(); // Expand to full height

// Get user info
const user = WebApp.initDataUnsafe.user;
if (user) {
  const userId = user.id; // Required for API calls
  const firstName = user.first_name;
  const lastName = user.last_name;
  const username = user.username;
  const languageCode = user.language_code; // e.g., 'en'
  const isPremium = user.is_premium; // Boolean
}
```

### Getting Shop Token from Deep Link

```typescript
// Shop token from URL parameter
const shopToken = WebApp.initDataUnsafe.start_param || 'default';

// Example: https://t.me/yourbot?start=my_shop_123
// Will give start_param = 'my_shop_123'
```

### Using Main Button

```typescript
// Show main button
WebApp.MainButton.setText('Complete Order');
WebApp.MainButton.show();
WebApp.MainButton.enable();

// Handle click
WebApp.MainButton.onClick(() => {
  // Show progress
  WebApp.MainButton.showProgress();
  
  // Make API call
  createOrder().then(() => {
    WebApp.MainButton.hideProgress();
    WebApp.showAlert('Order created!');
  });
});

// Change color
WebApp.MainButton.setParams({
  color: '#00FF00',
  text_color: '#FFFFFF'
});

// Hide when done
WebApp.MainButton.hide();
```

### Using Back Button

```typescript
// Show back button
WebApp.BackButton.show();

// Handle click
WebApp.BackButton.onClick(() => {
  window.history.back();
  // or navigate programmatically
});

// Hide
WebApp.BackButton.hide();
```

### Theme Colors

```typescript
// Access Telegram theme colors
const themeParams = WebApp.themeParams;

// Available colors:
const bgColor = themeParams.bg_color;
const textColor = themeParams.text_color;
const hintColor = themeParams.hint_color;
const linkColor = themeParams.link_color;
const buttonColor = themeParams.button_color;
const buttonTextColor = themeParams.button_text_color;
```

### Showing Alerts and Popups

```typescript
// Simple alert
WebApp.showAlert('Order created successfully!');

// Alert with callback
WebApp.showAlert('Are you sure?', (confirmed) => {
  if (confirmed) {
    // User clicked OK
  }
});

// Confirmation dialog
WebApp.showConfirm('Delete this item?', (confirmed) => {
  if (confirmed) {
    deleteItem();
  }
});

// Popup with custom buttons
WebApp.showPopup({
  title: 'Choose Action',
  message: 'What would you like to do?',
  buttons: [
    { id: 'edit', type: 'default', text: 'Edit' },
    { id: 'delete', type: 'destructive', text: 'Delete' },
    { type: 'cancel' }
  ]
}, (buttonId) => {
  if (buttonId === 'edit') {
    // Handle edit
  } else if (buttonId === 'delete') {
    // Handle delete
  }
});
```

### Deep Linking

Create deep links to your Mini App:

```
https://t.me/YOUR_BOT_USERNAME?start=SHOP_TOKEN
```

Examples:
- `https://t.me/myshopbot?start=coffee_shop_1234`
- `https://t.me/myshopbot?start=electronics_5678`

Users clicking these links will open your Mini App with the shop token available in `start_param`.

---

## Styling Guide

### CSS Variables for Telegram Theming

```css
/* src/styles/theme.css */

:root {
  /* Telegram theme colors - will be overridden by JS */
  --tg-theme-bg-color: #ffffff;
  --tg-theme-text-color: #000000;
  --tg-theme-hint-color: #999999;
  --tg-theme-link-color: #2481cc;
  --tg-theme-button-color: #2481cc;
  --tg-theme-button-text-color: #ffffff;
  --tg-theme-secondary-bg-color: #f4f4f5;
  
  /* Custom variables */
  --spacing-xs: 4px;
  --spacing-sm: 8px;
  --spacing-md: 16px;
  --spacing-lg: 24px;
  --spacing-xl: 32px;
  
  --border-radius: 12px;
  --border-radius-sm: 8px;
  
  --shadow: 0 2px 8px rgba(0, 0, 0, 0.1);
  --shadow-lg: 0 4px 16px rgba(0, 0, 0, 0.15);
}

body {
  margin: 0;
  padding: 0;
  font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', 'Roboto', 'Helvetica', 'Arial', sans-serif;
  background-color: var(--tg-theme-bg-color);
  color: var(--tg-theme-text-color);
}

* {
  box-sizing: border-box;
}
```

### Apply Theme Colors with JavaScript

```typescript
// src/lib/theme.ts

import WebApp from '@twa-dev/sdk';

export function applyTelegramTheme() {
  const theme = WebApp.themeParams;
  
  if (theme.bg_color) {
    document.documentElement.style.setProperty('--tg-theme-bg-color', theme.bg_color);
  }
  if (theme.text_color) {
    document.documentElement.style.setProperty('--tg-theme-text-color', theme.text_color);
  }
  if (theme.hint_color) {
    document.documentElement.style.setProperty('--tg-theme-hint-color', theme.hint_color);
  }
  if (theme.link_color) {
    document.documentElement.style.setProperty('--tg-theme-link-color', theme.link_color);
  }
  if (theme.button_color) {
    document.documentElement.style.setProperty('--tg-theme-button-color', theme.button_color);
  }
  if (theme.button_text_color) {
    document.documentElement.style.setProperty('--tg-theme-button-text-color', theme.button_text_color);
  }
  if (theme.secondary_bg_color) {
    document.documentElement.style.setProperty('--tg-theme-secondary-bg-color', theme.secondary_bg_color);
  }
}

// Call in your main App component
// useEffect(() => {
//   applyTelegramTheme();
// }, []);
```

### Component Styles

```css
/* Products Grid */
.products-grid {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(160px, 1fr));
  gap: var(--spacing-md);
  padding: var(--spacing-md);
}

.product-card {
  background: var(--tg-theme-secondary-bg-color);
  border-radius: var(--border-radius);
  padding: var(--spacing-md);
  box-shadow: var(--shadow);
}

.product-card img {
  width: 100%;
  height: 160px;
  object-fit: cover;
  border-radius: var(--border-radius-sm);
  margin-bottom: var(--spacing-sm);
}

.product-card h3 {
  font-size: 16px;
  margin: var(--spacing-sm) 0;
}

.product-card button {
  width: 100%;
  padding: var(--spacing-sm);
  background: var(--tg-theme-button-color);
  color: var(--tg-theme-button-text-color);
  border: none;
  border-radius: var(--border-radius-sm);
  font-size: 14px;
  cursor: pointer;
}

/* Cart Items */
.cart-item {
  display: flex;
  align-items: center;
  gap: var(--spacing-md);
  padding: var(--spacing-md);
  background: var(--tg-theme-secondary-bg-color);
  border-radius: var(--border-radius);
  margin-bottom: var(--spacing-sm);
}

.quantity-controls {
  display: flex;
  align-items: center;
  gap: var(--spacing-sm);
}

.quantity-controls button {
  width: 32px;
  height: 32px;
  border-radius: 50%;
  border: 1px solid var(--tg-theme-hint-color);
  background: transparent;
  color: var(--tg-theme-text-color);
  font-size: 18px;
  cursor: pointer;
}

/* Loading States */
.loading {
  display: flex;
  justify-content: center;
  align-items: center;
  min-height: 200px;
}

.spinner {
  width: 40px;
  height: 40px;
  border: 4px solid var(--tg-theme-hint-color);
  border-top-color: var(--tg-theme-button-color);
  border-radius: 50%;
  animation: spin 1s linear infinite;
}

@keyframes spin {
  to { transform: rotate(360deg); }
}

/* Empty States */
.empty-state {
  text-align: center;
  padding: var(--spacing-xl);
  color: var(--tg-theme-hint-color);
}

/* Status Badges */
.status {
  padding: 4px 12px;
  border-radius: 12px;
  font-size: 12px;
  font-weight: 600;
}

.status-pending {
  background: #fef3c7;
  color: #92400e;
}

.status-completed {
  background: #d1fae5;
  color: #065f46;
}

.status-cancelled {
  background: #fee2e2;
  color: #991b1b;
}
```

---

## Testing Guide

### Manual Browser Testing

Test the production API directly from your browser console or command line:

```bash
# Test 1: Check server is running
curl https://shopkit-backend.onrender.com/api/shops/token/test

# Expected: 404 JSON response (server is working, shop not found)
# Response: {"error":"Shop not found","code":"SHOP_NOT_FOUND"}

# Test 2: Create a test shop
curl -X POST https://shopkit-backend.onrender.com/api/shops \
  -H "Content-Type: application/json" \
  -d '{
    "owner_tg_id": 123456789,
    "title": "Test Shop",
    "description": "A test shop for development",
    "currency": "USD"
  }'

# Response: {"success":true,"shop":{"id":1,"shop_token":"test_shop_6789",...}}

# Test 3: Get the shop you just created
curl https://shopkit-backend.onrender.com/api/shops/token/test_shop_6789

# Test 4: Add a product (use the shop_id from response above)
curl -X POST https://shopkit-backend.onrender.com/api/admin/products \
  -H "Content-Type: application/json" \
  -H "X-Owner-TG-ID: 123456789" \
  -d '{
    "shop_id": 1,
    "title": "Test Product",
    "price": 1999,
    "currency": "USD",
    "description": "A test product",
    "stock_count": 10,
    "category": "Electronics"
  }'

# Test 5: Get products
curl https://shopkit-backend.onrender.com/api/products/1

# Test 6: Save cart
curl -X POST https://shopkit-backend.onrender.com/api/cart \
  -H "Content-Type: application/json" \
  -d '{
    "telegram_user_id": 123456789,
    "shop_id": 1,
    "items": [
      {"product_id":"PRODUCT_UUID_HERE","title":"Test Product","quantity":2,"price":1999}
    ]
  }'

# Test 7: Get cart
curl "https://shopkit-backend.onrender.com/api/cart/123456789?shop_id=1"

# Test 8: Create order
curl -X POST https://shopkit-backend.onrender.com/api/orders \
  -H "Content-Type: application/json" \
  -d '{
    "shop_id": 1,
    "telegram_user_id": 123456789,
    "customer_name": "John Doe",
    "customer_phone": "+1234567890",
    "items": [
      {"product_id":"PRODUCT_UUID_HERE","title":"Test Product","quantity":2,"price":1999}
    ],
    "total_price": 3998,
    "currency": "USD"
  }'

# Test 9: Get user orders (IMPORTANT: Returns from ALL shops!)
curl https://shopkit-backend.onrender.com/api/user/orders/123456789
```

### TypeScript Testing Examples

```typescript
// test-api.ts

import { api } from './lib/api';

async function testAPI() {
  console.log('Testing ShopKit API...\n');

  // Test 1: Get shop
  console.log('1. Testing getShop()...');
  try {
    const shop = await api.getShop('test_shop_6789');
    console.log('✅ Shop loaded:', shop.title);
  } catch (error) {
    console.log('❌ Shop not found (expected if shop doesn\'t exist)');
  }

  // Test 2: Get products
  console.log('\n2. Testing getProducts()...');
  try {
    const products = await api.getProducts(1);
    console.log(`✅ Loaded ${products.length} products`);
    products.forEach(p => console.log(`   - ${p.title}: ${p.price} ${p.currency}`));
  } catch (error) {
    console.log('❌ Failed to load products');
  }

  // Test 3: Cart operations
  console.log('\n3. Testing cart operations...');
  const userId = 123456789;
  const shopId = 1;

  try {
    // Save cart
    await api.saveCart(userId, shopId, [
      { product_id: 'test-uuid', title: 'Test Item', quantity: 2, price: 1000 }
    ]);
    console.log('✅ Cart saved');

    // Get cart
    const cart = await api.getCart(userId, shopId);
    console.log(`✅ Cart retrieved: ${cart.items.length} items`);

    // Clear cart
    await api.clearCart(userId, shopId);
    console.log('✅ Cart cleared');
  } catch (error) {
    console.log('❌ Cart operations failed:', error);
  }

  // Test 4: Get user orders
  console.log('\n4. Testing getUserOrders()...');
  try {
    const orders = await api.getUserOrders(userId);
    console.log(`✅ Loaded ${orders.length} orders from ALL shops`);
    orders.forEach(o => {
      console.log(`   - ${o.shop_title}: ${o.total_price} ${o.currency} (${o.status})`);
    });
  } catch (error) {
    console.log('❌ Failed to load orders');
  }

  console.log('\n✅ All tests completed!');
}

// Run tests
testAPI();
```

### Testing in Telegram Bot

1. **Create your bot** with [@BotFather](https://t.me/botfather)
2. **Set up Mini App:**
   ```
   /newapp
   Select your bot
   Enter app title
   Enter app description
   Upload 640x360 photo
   Upload demo GIF (optional)
   Enter your app URL: https://your-app.com
   Enter short name: myshop
   ```

3. **Test deep linking:**
   ```
   https://t.me/YOUR_BOT?start=test_shop_6789
   ```

4. **Debug in Telegram Desktop:**
   - Enable debugging: Settings → Advanced → Debug Mode
   - Right-click Mini App → Inspect Element
   - Use Chrome DevTools to debug

---

## Production Examples

### Complete Mini App Setup

```typescript
// src/main.tsx

import React from 'react';
import ReactDOM from 'react-dom/client';
import { BrowserRouter, Routes, Route } from 'react-router-dom';
import WebApp from '@twa-dev/sdk';

import { ShopPage } from './pages/ShopPage';
import { CartPage } from './pages/CartPage';
import { CheckoutPage } from './pages/CheckoutPage';
import { OrdersPage } from './pages/OrdersPage';
import { OrderDetailPage } from './pages/OrderDetailPage';

import { applyTelegramTheme } from './lib/theme';

import './styles/theme.css';
import './styles/main.css';

// Initialize Telegram WebApp
WebApp.ready();
WebApp.expand();
applyTelegramTheme();

// Disable vertical swipes (optional)
WebApp.disableVerticalSwipes();

ReactDOM.createRoot(document.getElementById('root')!).render(
  <React.StrictMode>
    <BrowserRouter>
      <Routes>
        <Route path="/" element={<ShopPage />} />
        <Route path="/cart" element={<CartPage />} />
        <Route path="/checkout" element={<CheckoutPage />} />
        <Route path="/orders" element={<OrdersPage />} />
        <Route path="/orders/:orderId" element={<OrderDetailPage />} />
      </Routes>
    </BrowserRouter>
  </React.StrictMode>
);
```

### Real Production Flow

```typescript
// Example: Complete shopping flow

import WebApp from '@twa-dev/sdk';
import { api } from './lib/api';

async function completeShoppingFlow() {
  // 1. Initialize
  WebApp.ready();
  const userId = WebApp.initDataUnsafe.user?.id || 0;
  const shopToken = WebApp.initDataUnsafe.start_param || 'demo';

  // 2. Load shop
  const shop = await api.getShop(shopToken);
  console.log(`Shopping at: ${shop.title}`);

  // 3. Load products
  const products = await api.getProducts(shop.id);
  console.log(`${products.length} products available`);

  // 4. Load saved cart
  const savedCart = await api.getCart(userId, shop.id);
  console.log(`Cart has ${savedCart.items.length} items`);

  // 5. Add item to cart
  const product = products[0];
  const cartItems = [
    ...savedCart.items,
    {
      product_id: product.product_id,
      title: product.title,
      quantity: 1,
      price: product.price,
    },
  ];
  await api.saveCart(userId, shop.id, cartItems);
  WebApp.showAlert(`${product.title} added to cart!`);

  // 6. Create order
  const user = WebApp.initDataUnsafe.user;
  const total = cartItems.reduce((sum, item) => sum + item.price * item.quantity, 0);

  const { order } = await api.createOrder({
    shop_id: shop.id,
    telegram_user_id: userId,
    customer_name: `${user?.first_name} ${user?.last_name || ''}`,
    items: cartItems,
    total_price: total,
    currency: shop.currency,
  });

  console.log(`Order created: ${order.order_id}`);

  // 7. Clear cart
  await api.clearCart(userId, shop.id);

  // 8. View all orders from ALL shops
  const allOrders = await api.getUserOrders(userId);
  console.log(`You have ${allOrders.length} orders across all shops`);
  allOrders.forEach(o => {
    console.log(`- ${o.shop_title}: ${o.total_price} ${o.currency}`);
  });
}
```

---

## Deployment Checklist

### Frontend Deployment

- [ ] **Install dependencies**
  ```bash
  npm install @twa-dev/sdk react-router-dom
  npm install -D @types/node
  ```

- [ ] **Add Telegram script to index.html**
  ```html
  <script src="https://telegram.org/js/telegram-web-app.js"></script>
  ```

- [ ] **Configure environment variables**
  ```env
  VITE_API_BASE_URL=https://shopkit-backend.onrender.com/api
  ```

- [ ] **Copy type definitions** from this guide

- [ ] **Copy API client** (`src/lib/api.ts`)

- [ ] **Copy hooks** (`src/hooks/`)

- [ ] **Copy theme utility** (`src/lib/theme.ts`)

- [ ] **Add CSS styles** (`src/styles/`)

- [ ] **Test locally**
  ```bash
  npm run dev
  # Open http://localhost:5173?tgWebAppStartParam=test_shop
  ```

- [ ] **Build for production**
  ```bash
  npm run build
  ```

- [ ] **Deploy to hosting** (Vercel, Netlify, GitHub Pages, etc.)

- [ ] **Configure Mini App URL in BotFather**

- [ ] **Test in Telegram** (both mobile and desktop)

- [ ] **Set up error monitoring** (Sentry, LogRocket, etc.)

### Common Hosting Options

**Vercel:**
```bash
npm install -g vercel
vercel login
vercel
```

**Netlify:**
```bash
npm install -g netlify-cli
netlify login
netlify deploy --prod
```

**GitHub Pages:**
```bash
npm run build
# Push dist/ folder to gh-pages branch
```

---

## Troubleshooting

### Common Issues

#### 1. CORS Errors

**Problem:** `Access to fetch has been blocked by CORS policy`

**Solution:** The backend already has CORS enabled for all origins. Make sure you're using the correct API URL:
```typescript
const API_BASE_URL = 'https://shopkit-backend.onrender.com/api';
```

#### 2. Shop Not Found (404)

**Problem:** `{"error":"Shop not found","code":"SHOP_NOT_FOUND"}`

**Solution:** 
- Verify the shop token is correct
- Create a shop first using the `/shops` endpoint
- Check if you're using the correct shop token from `start_param`

```typescript
// Debug shop token
const shopToken = WebApp.initDataUnsafe.start_param;
console.log('Shop token:', shopToken);
```

#### 3. Cart Not Syncing

**Problem:** Cart items disappear or don't save

**Solution:**
- Check that `telegram_user_id` is valid (not 0)
- Verify `shop_id` is correct
- Check browser console for errors
- Test cart API directly with curl

```typescript
// Debug cart saving
console.log('Saving cart:', { userId, shopId, items });
await api.saveCart(userId, shopId, items);
console.log('Cart saved successfully');
```

#### 4. Order Creation Failures

**Problem:** Order creation returns 400 or 500 error

**Solution:**
- Ensure all required fields are provided:
  - `shop_id` (number)
  - `telegram_user_id` (number)
  - `customer_name` (string)
  - `items` (array with at least one item)
  - `total_price` (number)
  - `currency` (string)
- Verify items array format matches `CartItem` interface
- Check that `total_price` matches sum of items

```typescript
// Validate before creating order
const total = items.reduce((sum, item) => sum + item.price * item.quantity, 0);
console.log('Order total:', total);
console.log('Order items:', items);
```

#### 5. Telegram WebApp Not Available

**Problem:** `WebApp is not defined` or `Cannot read property 'user' of undefined`

**Solution:**
- Add Telegram script to `index.html` BEFORE your app script
- Test in actual Telegram app (not regular browser)
- For local testing, use mock data:

```typescript
// src/lib/telegram-mock.ts
export const MockWebApp = {
  initDataUnsafe: {
    user: {
      id: 123456789,
      first_name: 'Test',
      last_name: 'User',
      username: 'testuser',
    },
    start_param: 'demo',
  },
  ready: () => console.log('[Mock] WebApp ready'),
  expand: () => console.log('[Mock] WebApp expanded'),
  MainButton: {
    setText: (text: string) => console.log('[Mock] MainButton text:', text),
    show: () => console.log('[Mock] MainButton shown'),
    hide: () => console.log('[Mock] MainButton hidden'),
    onClick: (fn: () => void) => console.log('[Mock] MainButton onClick registered'),
    offClick: (fn: () => void) => console.log('[Mock] MainButton offClick'),
    showProgress: () => console.log('[Mock] MainButton progress shown'),
    hideProgress: () => console.log('[Mock] MainButton progress hidden'),
  },
  BackButton: {
    show: () => console.log('[Mock] BackButton shown'),
    hide: () => console.log('[Mock] BackButton hidden'),
    onClick: (fn: () => void) => console.log('[Mock] BackButton onClick registered'),
  },
  showAlert: (message: string, callback?: () => void) => {
    alert(message);
    callback?.();
  },
  themeParams: {
    bg_color: '#ffffff',
    text_color: '#000000',
    hint_color: '#999999',
    link_color: '#2481cc',
    button_color: '#2481cc',
    button_text_color: '#ffffff',
  },
};

// Use mock in development
const WebApp = typeof window !== 'undefined' && (window as any).Telegram?.WebApp
  ? (window as any).Telegram.WebApp
  : MockWebApp;

export default WebApp;
```

#### 6. TypeScript Errors

**Problem:** Type errors in API client or hooks

**Solution:**
- Ensure all type definitions from this guide are copied
- Install missing types: `npm install -D @types/node @types/react`
- Check `tsconfig.json` settings match the guide

#### 7. Build Errors

**Problem:** Build fails with module not found

**Solution:**
```bash
# Clear cache and reinstall
rm -rf node_modules package-lock.json
npm install

# Clear Vite cache
rm -rf .vite

# Rebuild
npm run build
```

#### 8. Mobile Layout Issues

**Problem:** App doesn't fill screen or has scrolling issues

**Solution:**
```typescript
// Ensure WebApp is expanded
WebApp.ready();
WebApp.expand();

// Disable vertical swipes if needed
WebApp.disableVerticalSwipes();

// CSS fix
body {
  margin: 0;
  padding: 0;
  overflow-x: hidden;
  min-height: 100vh;
}
```

### Debug Mode

Enable verbose logging for debugging:

```typescript
// src/lib/api.ts - Add to APIClient class

private debug = true; // Set to false in production

private async request<T>(endpoint: string, options?: RequestInit): Promise<T> {
  if (this.debug) {
    console.log('[API] Request:', endpoint, options);
  }

  try {
    const response = await fetch(`${this.baseURL}${endpoint}`, options);
    const data = await response.json();

    if (this.debug) {
      console.log('[API] Response:', endpoint, data);
    }

    if (!response.ok) {
      throw new APIError(data.error || 'API Error', response.status);
    }

    return data;
  } catch (error) {
    if (this.debug) {
      console.error('[API] Error:', endpoint, error);
    }
    throw error;
  }
}
```

### Getting Help

1. **Check API Status:** https://shopkit-backend.onrender.com/api/shops/token/test
2. **Test with curl:** Use the examples in the Testing Guide
3. **Check browser console:** Look for error messages
4. **Test in Telegram Desktop:** Use Debug Mode for better error messages
5. **Review this guide:** Make sure you've followed all steps

---

## Summary

This guide provides everything you need to build a production-ready Telegram Mini App with ShopKit backend:

✅ **Complete TypeScript definitions** matching the database schema  
✅ **Full-featured API client** with error handling and retry logic  
✅ **Production-ready React hooks** with optimistic updates  
✅ **Ready-to-use page components** for all shopping flows  
✅ **Telegram WebApp integration** with all features  
✅ **Styling guide** with theme support  
✅ **Comprehensive testing** examples  
✅ **Troubleshooting** for common issues  

### Key Features Highlighted

🎯 **Multi-Shop Support:** Users can order from multiple shops  
🎯 **Order History:** `/user/orders` returns orders from ALL shops  
🎯 **Cart Persistence:** Server-side cart syncing with debouncing  
🎯 **Production Ready:** Uses actual production API at Render.com  
🎯 **Type Safe:** Full TypeScript coverage  

### Next Steps

1. **Start with the Quick Start** section
2. **Copy the API client and hooks** to your project
3. **Use the page components** as templates
4. **Test thoroughly** with the testing guide
5. **Deploy** following the checklist

Happy coding! 🚀
