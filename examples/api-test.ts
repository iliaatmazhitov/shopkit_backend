/**
 * API Testing Script
 * 
 * This script demonstrates how to test the ShopKit API
 * Run with: npx ts-node api-test.ts
 */

const API_BASE = 'https://shopkit-backend.onrender.com/api';

interface CartItem {
  product_id: string;
  title: string;
  quantity: number;
  price: number;
}

async function request<T>(endpoint: string, options?: RequestInit): Promise<T> {
  const response = await fetch(`${API_BASE}${endpoint}`, {
    ...options,
    headers: {
      'Content-Type': 'application/json',
      ...options?.headers,
    },
  });

  if (!response.ok) {
    const error = await response.json().catch(() => ({}));
    throw new Error(error.error || `HTTP ${response.status}`);
  }

  return response.json();
}

async function testAPI() {
  console.log('🚀 Testing ShopKit Production API\n');
  console.log('API Base:', API_BASE, '\n');

  const testUserId = Date.now(); // Unique test user ID
  let testShopId = 0;
  let testProductId = '';

  try {
    // Test 1: Create a shop
    console.log('1️⃣  Creating test shop...');
    const { shop } = await request<any>('/shops', {
      method: 'POST',
      body: JSON.stringify({
        owner_tg_id: testUserId,
        title: 'Test Coffee Shop',
        description: 'A test shop for API testing',
        currency: 'USD',
      }),
    });
    testShopId = shop.id;
    console.log(`✅ Shop created: "${shop.title}" (ID: ${shop.id}, Token: ${shop.shop_token})\n`);

    // Test 2: Get shop by token
    console.log('2️⃣  Getting shop by token...');
    const fetchedShop = await request<any>(`/shops/token/${shop.shop_token}`);
    console.log(`✅ Shop loaded: ${fetchedShop.title}\n`);

    // Test 3: Create a product
    console.log('3️⃣  Creating test product...');
    const { product } = await request<any>('/admin/products', {
      method: 'POST',
      headers: {
        'X-Owner-TG-ID': testUserId.toString(),
      },
      body: JSON.stringify({
        shop_id: testShopId,
        title: 'Espresso',
        price: 350,
        currency: 'USD',
        description: 'Rich and bold espresso',
        stock_count: 100,
        category: 'Coffee',
      }),
    });
    testProductId = product.product_id;
    console.log(`✅ Product created: "${product.title}" (ID: ${product.product_id})\n`);

    // Test 4: Get products
    console.log('4️⃣  Getting all products...');
    const products = await request<any[]>(`/products/${testShopId}`);
    console.log(`✅ Loaded ${products.length} products\n`);

    // Test 5: Save cart
    console.log('5️⃣  Saving cart...');
    const cartItems: CartItem[] = [
      {
        product_id: testProductId,
        title: 'Espresso',
        quantity: 2,
        price: 350,
      },
    ];
    await request('/cart', {
      method: 'POST',
      body: JSON.stringify({
        telegram_user_id: testUserId,
        shop_id: testShopId,
        items: cartItems,
      }),
    });
    console.log('✅ Cart saved\n');

    // Test 6: Get cart
    console.log('6️⃣  Getting cart...');
    const cart = await request<any>(`/cart/${testUserId}?shop_id=${testShopId}`);
    console.log(`✅ Cart loaded: ${cart.items.length} items\n`);

    // Test 7: Create order
    console.log('7️⃣  Creating order...');
    const { order } = await request<any>('/orders', {
      method: 'POST',
      body: JSON.stringify({
        shop_id: testShopId,
        telegram_user_id: testUserId,
        customer_name: 'Test Customer',
        customer_phone: '+1234567890',
        items: cartItems,
        total_price: 700,
        currency: 'USD',
      }),
    });
    console.log(`✅ Order created: ${order.order_id}\n`);

    // Test 8: Get user orders
    console.log('8️⃣  Getting user orders (from ALL shops)...');
    const orders = await request<any[]>(`/user/orders/${testUserId}`);
    console.log(`✅ Loaded ${orders.length} orders`);
    orders.forEach((o, i) => {
      console.log(`   ${i + 1}. ${o.shop_title} - ${o.total_price} ${o.currency} (${o.status})`);
    });
    console.log();

    // Test 9: Update product
    console.log('9️⃣  Updating product...');
    await request(`/admin/products/${testProductId}`, {
      method: 'PUT',
      headers: {
        'X-Owner-TG-ID': testUserId.toString(),
      },
      body: JSON.stringify({
        price: 400,
        stock_count: 90,
      }),
    });
    console.log('✅ Product updated\n');

    // Test 10: Clear cart
    console.log('🔟 Clearing cart...');
    await request(`/cart/${testUserId}?shop_id=${testShopId}`, {
      method: 'DELETE',
    });
    console.log('✅ Cart cleared\n');

    console.log('✅ All tests passed! 🎉\n');
    console.log('Test Results:');
    console.log(`  - Shop ID: ${testShopId}`);
    console.log(`  - Shop Token: ${shop.shop_token}`);
    console.log(`  - Product ID: ${testProductId}`);
    console.log(`  - Order ID: ${order.order_id}`);
    console.log(`  - User ID: ${testUserId}`);

  } catch (error) {
    console.error('❌ Test failed:', error);
    process.exit(1);
  }
}

// Run tests
testAPI();
