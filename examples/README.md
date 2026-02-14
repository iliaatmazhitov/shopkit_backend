# ShopKit Frontend Examples

This directory contains practical examples for integrating with the ShopKit backend API.

## Files

### 1. `api-test.ts`
Complete API testing script that demonstrates all endpoints.

**Usage:**
```bash
npx ts-node api-test.ts
```

**What it does:**
- Creates a test shop
- Adds products
- Manages cart operations
- Creates orders
- Fetches user order history

### 2. `telegram-init.ts`
Telegram Mini App initialization utilities.

**Features:**
- App initialization (ready, expand)
- User info extraction
- Shop token from deep links
- Theme color application
- Back button setup
- Main button setup
- Alert and confirmation helpers

**Usage:**
```typescript
import { completeSetup, setupMainButton } from './telegram-init';

function App() {
  useEffect(() => {
    const { user, shopToken } = completeSetup();
    // Your app logic here
  }, []);
}
```

### 3. `cart-example.tsx`
Complete cart implementation with React hooks.

**Features:**
- Server-side cart synchronization
- Debounced saves (500ms)
- Optimistic updates
- Quantity controls
- Add/remove items
- Total calculation

**Usage:**
```tsx
import { useCart, CartComponent } from './cart-example';

function MyApp() {
  return <CartComponent userId={123456789} shopId={1} currency="USD" />;
}
```

### 4. `checkout-flow.tsx`
Full checkout page implementation.

**Features:**
- Form validation
- Telegram MainButton integration
- Order creation
- Cart clearing
- Error handling
- Success confirmation

**Usage:**
```tsx
import CheckoutPage from './checkout-flow';

function App() {
  return (
    <CheckoutPage
      userId={userId}
      shop={shop}
      cartItems={cartItems}
      onSuccess={() => navigate('/orders')}
    />
  );
}
```

## Quick Start

1. **Install dependencies:**
```bash
npm install @twa-dev/sdk react react-dom
npm install -D typescript @types/react @types/react-dom
```

2. **Copy the files** you need to your project

3. **Update API_BASE** if using a different environment:
```typescript
const API_BASE = 'https://shopkit-backend.onrender.com/api';
```

4. **Test the API:**
```bash
npx ts-node api-test.ts
```

## Integration Tips

### Using with React Router

```tsx
import { BrowserRouter, Routes, Route } from 'react-router-dom';
import { ShopPage } from './pages/ShopPage';
import { CartComponent } from './examples/cart-example';
import CheckoutPage from './examples/checkout-flow';

function App() {
  return (
    <BrowserRouter>
      <Routes>
        <Route path="/" element={<ShopPage />} />
        <Route path="/cart" element={<CartComponent {...props} />} />
        <Route path="/checkout" element={<CheckoutPage {...props} />} />
      </Routes>
    </BrowserRouter>
  );
}
```

### Environment Variables

Create `.env` file:
```env
VITE_API_BASE_URL=https://shopkit-backend.onrender.com/api
```

Use in code:
```typescript
const API_BASE = import.meta.env.VITE_API_BASE_URL;
```

## Production Checklist

- [ ] Copy examples to your project
- [ ] Update API_BASE URL
- [ ] Test in Telegram app
- [ ] Handle errors gracefully
- [ ] Add loading states
- [ ] Implement analytics
- [ ] Test on mobile devices
- [ ] Deploy to production

## More Information

See [FRONTEND_COMPLETE_GUIDE.md](../FRONTEND_COMPLETE_GUIDE.md) for the complete integration guide.
