#!/bin/bash

# ShopKit API Test Script
# This script tests all the main API endpoints

# Configuration
API_BASE="${API_BASE:-http://localhost:8080/api}"
TEST_TELEGRAM_USER_ID=123456789
TEST_SHOP_TOKEN="test_shop_123"

echo "========================================="
echo "ShopKit API Test Suite"
echo "========================================="
echo "API Base: $API_BASE"
echo ""

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test counter
TESTS_PASSED=0
TESTS_FAILED=0

# Function to test an endpoint
test_endpoint() {
    local name="$1"
    local method="$2"
    local endpoint="$3"
    local data="$4"
    
    echo -e "${YELLOW}Testing: $name${NC}"
    
    if [ -n "$data" ]; then
        response=$(curl -s -w "\n%{http_code}" -X "$method" "$API_BASE$endpoint" \
            -H "Content-Type: application/json" \
            -d "$data")
    else
        response=$(curl -s -w "\n%{http_code}" -X "$method" "$API_BASE$endpoint")
    fi
    
    http_code=$(echo "$response" | tail -n 1)
    body=$(echo "$response" | head -n -1)
    
    if [ "$http_code" -ge 200 ] && [ "$http_code" -lt 300 ]; then
        echo -e "${GREEN}✓ PASSED${NC} (HTTP $http_code)"
        echo "$body" | jq . 2>/dev/null || echo "$body"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗ FAILED${NC} (HTTP $http_code)"
        echo "$body"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    echo ""
}

echo "========================================="
echo "1. Shop Endpoints"
echo "========================================="

# Test get shop (will likely fail if shop doesn't exist, that's OK)
test_endpoint "Get Shop by Token" "GET" "/shops/token/$TEST_SHOP_TOKEN"

# Test create shop
SHOP_DATA='{
  "owner_tg_id": '"$TEST_TELEGRAM_USER_ID"',
  "title": "Test Coffee Shop",
  "description": "A test shop for API testing",
  "currency": "RUB",
  "shop_token": "'"$TEST_SHOP_TOKEN"'"
}'
test_endpoint "Create Shop" "POST" "/shops" "$SHOP_DATA"

# Get shop again to extract ID
echo "Fetching shop ID..."
SHOP_RESPONSE=$(curl -s "$API_BASE/shops/token/$TEST_SHOP_TOKEN")
SHOP_ID=$(echo "$SHOP_RESPONSE" | jq -r '.id' 2>/dev/null)
echo "Shop ID: $SHOP_ID"
echo ""

if [ -z "$SHOP_ID" ] || [ "$SHOP_ID" = "null" ]; then
    echo -e "${RED}WARNING: Could not get shop ID. Some tests may fail.${NC}"
    SHOP_ID=1
fi

echo "========================================="
echo "2. Product Endpoints"
echo "========================================="

# Test get products (may be empty)
test_endpoint "Get Products" "GET" "/products/$SHOP_ID"

echo "========================================="
echo "3. Cart Endpoints"
echo "========================================="

# Test save cart
CART_DATA='{
  "telegram_user_id": '"$TEST_TELEGRAM_USER_ID"',
  "shop_id": '"$SHOP_ID"',
  "items": [
    {
      "product_id": "00000000-0000-0000-0000-000000000001",
      "title": "Coffee",
      "quantity": 2,
      "price": 250
    },
    {
      "product_id": "00000000-0000-0000-0000-000000000002",
      "title": "Tea",
      "quantity": 1,
      "price": 150
    }
  ]
}'
test_endpoint "Save Cart" "POST" "/cart" "$CART_DATA"

# Test get cart
test_endpoint "Get Cart" "GET" "/cart/$TEST_TELEGRAM_USER_ID?shop_id=$SHOP_ID"

echo "========================================="
echo "4. Order Endpoints"
echo "========================================="

# Test create order
ORDER_DATA='{
  "shop_id": '"$SHOP_ID"',
  "telegram_user_id": '"$TEST_TELEGRAM_USER_ID"',
  "customer_name": "John Doe",
  "customer_phone": "+1234567890",
  "items": [
    {
      "product_id": "00000000-0000-0000-0000-000000000001",
      "title": "Coffee",
      "quantity": 2,
      "price": 250
    }
  ],
  "total_price": 500,
  "currency": "RUB"
}'
test_endpoint "Create Order" "POST" "/orders" "$ORDER_DATA"

# Test get user orders
test_endpoint "Get User Orders" "GET" "/user/orders/$TEST_TELEGRAM_USER_ID"

echo "========================================="
echo "5. Cart Cleanup"
echo "========================================="

# Test clear cart
test_endpoint "Clear Cart" "DELETE" "/cart/$TEST_TELEGRAM_USER_ID?shop_id=$SHOP_ID"

# Verify cart is cleared
test_endpoint "Verify Cart Cleared" "GET" "/cart/$TEST_TELEGRAM_USER_ID?shop_id=$SHOP_ID"

echo "========================================="
echo "Test Summary"
echo "========================================="
echo -e "Tests Passed: ${GREEN}$TESTS_PASSED${NC}"
echo -e "Tests Failed: ${RED}$TESTS_FAILED${NC}"
echo "========================================="

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed.${NC}"
    exit 1
fi
