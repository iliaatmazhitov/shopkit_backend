/**
 * Cart Implementation Example
 * 
 * This example shows a complete cart implementation with:
 * - Server synchronization
 * - Debounced saves
 * - Optimistic updates
 * - Quantity controls
 */

import { useState, useEffect, useCallback, useRef } from 'react';

const API_BASE = 'https://shopkit-backend.onrender.com/api';
const DEBOUNCE_DELAY = 500; // 500ms

interface CartItem {
  product_id: string;
  title: string;
  quantity: number;
  price: number;
}

interface Product {
  product_id: string;
  title: string;
  price: number;
  currency: string;
  stock_count: number;
}

// Custom hook for cart management
export function useCart(userId: number, shopId: number) {
  const [items, setItems] = useState<CartItem[]>([]);
  const [loading, setLoading] = useState(true);
  const [saving, setSaving] = useState(false);
  const saveTimeoutRef = useRef<NodeJS.Timeout>();

  // Load cart from server
  const loadCart = useCallback(async () => {
    try {
      setLoading(true);
      const response = await fetch(`${API_BASE}/cart/${userId}?shop_id=${shopId}`);
      if (response.ok) {
        const data = await response.json();
        setItems(data.items || []);
      } else {
        setItems([]);
      }
    } catch (error) {
      console.error('Failed to load cart:', error);
      setItems([]);
    } finally {
      setLoading(false);
    }
  }, [userId, shopId]);

  // Save cart to server (debounced)
  const saveCart = useCallback(
    (newItems: CartItem[]) => {
      // Clear existing timeout
      if (saveTimeoutRef.current) {
        clearTimeout(saveTimeoutRef.current);
      }

      // Debounce the save
      saveTimeoutRef.current = setTimeout(async () => {
        try {
          setSaving(true);
          await fetch(`${API_BASE}/cart`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
              telegram_user_id: userId,
              shop_id: shopId,
              items: newItems,
            }),
          });
        } catch (error) {
          console.error('Failed to save cart:', error);
        } finally {
          setSaving(false);
        }
      }, DEBOUNCE_DELAY);
    },
    [userId, shopId]
  );

  // Add item to cart
  const addItem = useCallback(
    (product: Product, quantity = 1) => {
      setItems((current) => {
        const existing = current.find((item) => item.product_id === product.product_id);

        let newItems: CartItem[];
        if (existing) {
          newItems = current.map((item) =>
            item.product_id === product.product_id
              ? { ...item, quantity: item.quantity + quantity }
              : item
          );
        } else {
          newItems = [
            ...current,
            {
              product_id: product.product_id,
              title: product.title,
              price: product.price,
              quantity,
            },
          ];
        }

        saveCart(newItems);
        return newItems;
      });
    },
    [saveCart]
  );

  // Remove item from cart
  const removeItem = useCallback(
    (productId: string) => {
      setItems((current) => {
        const newItems = current.filter((item) => item.product_id !== productId);
        saveCart(newItems);
        return newItems;
      });
    },
    [saveCart]
  );

  // Update quantity
  const updateQuantity = useCallback(
    (productId: string, quantity: number) => {
      if (quantity <= 0) {
        removeItem(productId);
        return;
      }

      setItems((current) => {
        const newItems = current.map((item) =>
          item.product_id === productId ? { ...item, quantity } : item
        );
        saveCart(newItems);
        return newItems;
      });
    },
    [saveCart, removeItem]
  );

  // Clear cart
  const clear = useCallback(async () => {
    try {
      await fetch(`${API_BASE}/cart/${userId}?shop_id=${shopId}`, {
        method: 'DELETE',
      });
      setItems([]);
    } catch (error) {
      console.error('Failed to clear cart:', error);
    }
  }, [userId, shopId]);

  // Calculate totals
  const total = items.reduce((sum, item) => sum + item.price * item.quantity, 0);
  const itemCount = items.reduce((sum, item) => sum + item.quantity, 0);

  // Load cart on mount
  useEffect(() => {
    loadCart();
  }, [loadCart]);

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
    saving,
    total,
    itemCount,
    addItem,
    removeItem,
    updateQuantity,
    clear,
    refetch: loadCart,
  };
}

// Example Cart Component
export function CartComponent({ userId, shopId, currency }: { userId: number; shopId: number; currency: string }) {
  const {
    items,
    loading,
    saving,
    total,
    itemCount,
    updateQuantity,
    removeItem,
    clear,
  } = useCart(userId, shopId);

  if (loading) {
    return <div className="loading">Loading cart...</div>;
  }

  if (items.length === 0) {
    return (
      <div className="empty-cart">
        <h2>Your cart is empty</h2>
        <p>Add some products to get started!</p>
      </div>
    );
  }

  return (
    <div className="cart">
      <div className="cart-header">
        <h2>Shopping Cart ({itemCount} items)</h2>
        {saving && <span className="saving-indicator">Saving...</span>}
      </div>

      <div className="cart-items">
        {items.map((item) => (
          <div key={item.product_id} className="cart-item">
            <div className="item-info">
              <h3>{item.title}</h3>
              <p className="item-price">
                {item.price} {currency} × {item.quantity}
              </p>
            </div>

            <div className="item-controls">
              <div className="quantity-controls">
                <button
                  onClick={() => updateQuantity(item.product_id, item.quantity - 1)}
                  aria-label="Decrease quantity"
                >
                  −
                </button>
                <span>{item.quantity}</span>
                <button
                  onClick={() => updateQuantity(item.product_id, item.quantity + 1)}
                  aria-label="Increase quantity"
                >
                  +
                </button>
              </div>

              <button
                className="remove-btn"
                onClick={() => removeItem(item.product_id)}
                aria-label="Remove item"
              >
                Remove
              </button>
            </div>

            <div className="item-total">
              {item.price * item.quantity} {currency}
            </div>
          </div>
        ))}
      </div>

      <div className="cart-footer">
        <button className="clear-cart-btn" onClick={clear}>
          Clear Cart
        </button>

        <div className="cart-total">
          <span>Total:</span>
          <span className="total-amount">
            {total} {currency}
          </span>
        </div>

        <button className="checkout-btn" onClick={() => console.log('Proceed to checkout')}>
          Checkout
        </button>
      </div>

      <style jsx>{`
        .cart {
          padding: 16px;
        }

        .cart-header {
          display: flex;
          justify-content: space-between;
          align-items: center;
          margin-bottom: 16px;
        }

        .saving-indicator {
          font-size: 12px;
          color: #999;
        }

        .cart-items {
          display: flex;
          flex-direction: column;
          gap: 12px;
          margin-bottom: 16px;
        }

        .cart-item {
          display: flex;
          gap: 16px;
          padding: 12px;
          background: #f5f5f5;
          border-radius: 8px;
        }

        .item-info {
          flex: 1;
        }

        .item-info h3 {
          margin: 0 0 4px 0;
          font-size: 16px;
        }

        .item-price {
          margin: 0;
          font-size: 14px;
          color: #666;
        }

        .item-controls {
          display: flex;
          flex-direction: column;
          gap: 8px;
        }

        .quantity-controls {
          display: flex;
          align-items: center;
          gap: 8px;
        }

        .quantity-controls button {
          width: 28px;
          height: 28px;
          border-radius: 50%;
          border: 1px solid #ccc;
          background: white;
          cursor: pointer;
          font-size: 16px;
        }

        .remove-btn {
          padding: 4px 8px;
          border: none;
          background: #ef4444;
          color: white;
          border-radius: 4px;
          cursor: pointer;
          font-size: 12px;
        }

        .item-total {
          font-weight: 600;
          white-space: nowrap;
        }

        .cart-footer {
          display: flex;
          flex-direction: column;
          gap: 12px;
          padding-top: 16px;
          border-top: 2px solid #ddd;
        }

        .cart-total {
          display: flex;
          justify-content: space-between;
          font-size: 18px;
          font-weight: 600;
        }

        .checkout-btn,
        .clear-cart-btn {
          padding: 12px;
          border: none;
          border-radius: 8px;
          font-size: 16px;
          font-weight: 600;
          cursor: pointer;
        }

        .checkout-btn {
          background: #2481cc;
          color: white;
        }

        .clear-cart-btn {
          background: #f5f5f5;
          color: #666;
        }

        .empty-cart {
          text-align: center;
          padding: 48px 16px;
          color: #666;
        }
      `}</style>
    </div>
  );
}

export default CartComponent;
