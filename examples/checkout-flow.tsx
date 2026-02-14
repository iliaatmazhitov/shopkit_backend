/**
 * Complete Checkout Flow Example
 * 
 * This example demonstrates a full checkout process:
 * - Form validation
 * - Order creation
 * - Cart clearing
 * - Telegram MainButton integration
 * - Error handling
 */

import { useState, useEffect } from 'react';
import WebApp from '@twa-dev/sdk';

const API_BASE = 'https://shopkit-backend.onrender.com/api';

interface CartItem {
  product_id: string;
  title: string;
  quantity: number;
  price: number;
}

interface Shop {
  id: number;
  title: string;
  currency: string;
}

interface CheckoutFormData {
  customerName: string;
  customerPhone: string;
  notes: string;
}

// Checkout Page Component
export function CheckoutPage({
  userId,
  shop,
  cartItems,
  onSuccess,
}: {
  userId: number;
  shop: Shop;
  cartItems: CartItem[];
  onSuccess: () => void;
}) {
  const [formData, setFormData] = useState<CheckoutFormData>({
    customerName: '',
    customerPhone: '',
    notes: '',
  });
  const [submitting, setSubmitting] = useState(false);
  const [error, setError] = useState<string | null>(null);

  // Calculate total
  const total = cartItems.reduce((sum, item) => sum + item.price * item.quantity, 0);

  // Initialize with user data
  useEffect(() => {
    const user = WebApp.initDataUnsafe.user;
    if (user) {
      setFormData((prev) => ({
        ...prev,
        customerName: `${user.first_name} ${user.last_name || ''}`.trim(),
      }));
    }
  }, []);

  // Setup Telegram buttons
  useEffect(() => {
    // Show back button
    WebApp.BackButton.show();
    WebApp.BackButton.onClick(() => {
      window.history.back();
    });

    // Setup main button
    WebApp.MainButton.setText(`Pay ${total} ${shop.currency}`);
    WebApp.MainButton.show();
    WebApp.MainButton.enable();
    WebApp.MainButton.onClick(handleSubmit);

    return () => {
      WebApp.BackButton.hide();
      WebApp.MainButton.hide();
      WebApp.MainButton.offClick(handleSubmit);
    };
  }, [total, shop.currency]);

  // Validate form
  const validateForm = (): boolean => {
    if (!formData.customerName.trim()) {
      setError('Please enter your name');
      WebApp.showAlert('Please enter your name');
      return false;
    }

    if (cartItems.length === 0) {
      setError('Your cart is empty');
      WebApp.showAlert('Your cart is empty');
      return false;
    }

    return true;
  };

  // Handle order submission
  const handleSubmit = async () => {
    if (!validateForm()) {
      return;
    }

    try {
      setSubmitting(true);
      setError(null);
      WebApp.MainButton.showProgress();

      // Create order
      const response = await fetch(`${API_BASE}/orders`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          shop_id: shop.id,
          telegram_user_id: userId,
          customer_name: formData.customerName,
          customer_phone: formData.customerPhone || undefined,
          items: cartItems,
          total_price: total,
          currency: shop.currency,
          notes: formData.notes || undefined,
        }),
      });

      if (!response.ok) {
        const errorData = await response.json();
        throw new Error(errorData.error || 'Failed to create order');
      }

      const { order } = await response.json();

      // Clear cart
      await fetch(`${API_BASE}/cart/${userId}?shop_id=${shop.id}`, {
        method: 'DELETE',
      });

      WebApp.MainButton.hideProgress();

      // Show success message
      WebApp.showAlert(
        `✅ Order created successfully!\n\nOrder ID: ${order.order_id.slice(0, 8)}...\nTotal: ${total} ${shop.currency}`,
        () => {
          onSuccess();
        }
      );
    } catch (err) {
      WebApp.MainButton.hideProgress();
      const errorMessage = err instanceof Error ? err.message : 'Failed to create order';
      setError(errorMessage);
      WebApp.showAlert(`❌ Error: ${errorMessage}`);
    } finally {
      setSubmitting(false);
    }
  };

  // Handle form field changes
  const handleChange = (field: keyof CheckoutFormData, value: string) => {
    setFormData((prev) => ({ ...prev, [field]: value }));
    setError(null);
  };

  if (cartItems.length === 0) {
    return (
      <div className="empty-checkout">
        <h2>Your cart is empty</h2>
        <button onClick={() => (window.location.href = '/')}>Go Shopping</button>
      </div>
    );
  }

  return (
    <div className="checkout-page">
      <h1>Checkout</h1>

      {error && (
        <div className="error-message">
          <span>⚠️</span>
          <p>{error}</p>
        </div>
      )}

      {/* Order Summary */}
      <section className="order-summary">
        <h2>Order Summary</h2>
        <div className="summary-items">
          {cartItems.map((item) => (
            <div key={item.product_id} className="summary-item">
              <div className="item-details">
                <span className="item-name">{item.title}</span>
                <span className="item-qty">× {item.quantity}</span>
              </div>
              <span className="item-price">
                {item.price * item.quantity} {shop.currency}
              </span>
            </div>
          ))}
        </div>
        <div className="summary-total">
          <strong>Total:</strong>
          <strong className="total-amount">
            {total} {shop.currency}
          </strong>
        </div>
      </section>

      {/* Customer Information Form */}
      <section className="checkout-form">
        <h2>Customer Information</h2>

        <div className="form-group">
          <label htmlFor="name">
            Full Name <span className="required">*</span>
          </label>
          <input
            id="name"
            type="text"
            value={formData.customerName}
            onChange={(e) => handleChange('customerName', e.target.value)}
            placeholder="John Doe"
            required
            disabled={submitting}
          />
        </div>

        <div className="form-group">
          <label htmlFor="phone">Phone Number (optional)</label>
          <input
            id="phone"
            type="tel"
            value={formData.customerPhone}
            onChange={(e) => handleChange('customerPhone', e.target.value)}
            placeholder="+1234567890"
            disabled={submitting}
          />
        </div>

        <div className="form-group">
          <label htmlFor="notes">Order Notes (optional)</label>
          <textarea
            id="notes"
            value={formData.notes}
            onChange={(e) => handleChange('notes', e.target.value)}
            placeholder="Any special requests or delivery instructions?"
            rows={3}
            disabled={submitting}
          />
        </div>
      </section>

      {/* Instructions */}
      <div className="checkout-hint">
        <p>💡 Click the button below to complete your order</p>
      </div>

      <style jsx>{`
        .checkout-page {
          padding: 16px;
          max-width: 600px;
          margin: 0 auto;
          padding-bottom: 80px; /* Space for MainButton */
        }

        h1 {
          margin: 0 0 16px 0;
          font-size: 24px;
        }

        h2 {
          font-size: 18px;
          margin: 0 0 12px 0;
        }

        .error-message {
          display: flex;
          gap: 8px;
          padding: 12px;
          background: #fee;
          border: 1px solid #fcc;
          border-radius: 8px;
          margin-bottom: 16px;
          color: #c00;
        }

        .order-summary {
          background: #f9f9f9;
          padding: 16px;
          border-radius: 12px;
          margin-bottom: 24px;
        }

        .summary-items {
          display: flex;
          flex-direction: column;
          gap: 8px;
          margin-bottom: 12px;
        }

        .summary-item {
          display: flex;
          justify-content: space-between;
          align-items: center;
        }

        .item-details {
          display: flex;
          gap: 8px;
        }

        .item-name {
          font-weight: 500;
        }

        .item-qty {
          color: #666;
        }

        .summary-total {
          display: flex;
          justify-content: space-between;
          padding-top: 12px;
          border-top: 2px solid #ddd;
          font-size: 18px;
        }

        .total-amount {
          color: #2481cc;
        }

        .checkout-form {
          background: white;
          padding: 16px;
          border-radius: 12px;
          margin-bottom: 16px;
        }

        .form-group {
          margin-bottom: 16px;
        }

        .form-group label {
          display: block;
          margin-bottom: 6px;
          font-weight: 500;
        }

        .required {
          color: #e00;
        }

        .form-group input,
        .form-group textarea {
          width: 100%;
          padding: 10px;
          border: 1px solid #ddd;
          border-radius: 8px;
          font-size: 16px;
          font-family: inherit;
        }

        .form-group input:focus,
        .form-group textarea:focus {
          outline: none;
          border-color: #2481cc;
        }

        .form-group input:disabled,
        .form-group textarea:disabled {
          background: #f5f5f5;
          color: #999;
        }

        .checkout-hint {
          text-align: center;
          padding: 16px;
          background: #e8f4f8;
          border-radius: 8px;
          color: #2481cc;
        }

        .checkout-hint p {
          margin: 0;
        }

        .empty-checkout {
          text-align: center;
          padding: 48px 16px;
        }

        .empty-checkout button {
          padding: 12px 24px;
          background: #2481cc;
          color: white;
          border: none;
          border-radius: 8px;
          font-size: 16px;
          cursor: pointer;
          margin-top: 16px;
        }
      `}</style>
    </div>
  );
}

export default CheckoutPage;
