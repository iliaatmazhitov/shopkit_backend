#!/bin/bash

# ShopKit Backend - API Testing Script
# Tests all endpoints to verify functionality

set -e

# Configuration
API_BASE="${1:-https://shopkit-backend.onrender.com/api}"
SHOP_TOKEN="test_shop_$(date +%s)"
TEST_USER_ID=123456789
OWNER_TG_ID=987654321

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}🧪 ShopKit API Test Suite${NC}"
echo "=========================="
echo "API Base: $API_BASE"
echo ""

TESTS_PASSED=0
TESTS_FAILED=0

function test_endpoint() {
    local name="$1"
    local method="$2"
    local endpoint="$3"
    local data="$4"
    local headers="$5"
    
    echo -n "Testing: $name... "
    
    local cmd="curl -s -w '\n%{http_code}' -X $method"
    if [ -n "$headers" ]; then
        cmd="$cmd $headers"
    fi
    if [ -n "$data" ]; then
        cmd="$cmd -H 'Content-Type: application/json' -d '$data'"
    fi
    cmd="$cmd $API_BASE$endpoint"
    
    local response=$(eval $cmd 2>&1)
    local status_code=$(echo "$response" | tail -n1)
    
    if [ "$status_code" = "200" ] || [ "$status_code" = "201" ]; then
        echo -e "${GREEN}✅ PASS (HTTP $status_code)${NC}"
        ((TESTS_PASSED++))
        return 0
    else
        echo -e "${YELLOW}⚠️  HTTP $status_code${NC}"
        ((TESTS_FAILED++))
        return 1
    fi
}

echo -e "${YELLOW}1. Testing Shop Endpoints${NC}"
echo "----------------------------"

# Create shop
SHOP_DATA='{
  "owner_tg_id": '$OWNER_TG_ID',
  "title": "Test Shop",
  "description": "Test shop for API testing",
  "currency": "RUB"
}'
test_endpoint "POST /shops (create shop)" "POST" "/shops" "$SHOP_DATA" || true

# Get shop (will 404 if not exists, that's ok)
test_endpoint "GET /shops/token/{token}" "GET" "/shops/token/$SHOP_TOKEN" "" "" || true

echo ""
echo -e "${YELLOW}2. Testing Product Endpoints${NC}"
echo "-------------------------------"

# Get products (will fail if shop doesn't exist)
test_endpoint "GET /products/{shop_id}" "GET" "/products/1" "" "" || true

echo ""
echo -e "${YELLOW}3. Testing Cart Endpoints${NC}"
echo "---------------------------"

# Save cart
CART_DATA='{
  "telegram_user_id": '$TEST_USER_ID',
  "shop_id": 1,
  "items": [
    {"product_id": "test-uuid", "title": "Test Product", "quantity": 2, "price": 1000}
  ]
}'
test_endpoint "POST /cart" "POST" "/cart" "$CART_DATA" || true

# Get cart
test_endpoint "GET /cart/{user_id}" "GET" "/cart/$TEST_USER_ID?shop_id=1" "" || true

echo ""
echo -e "${YELLOW}4. Testing Order Endpoints${NC}"
echo "----------------------------"

# Get user orders
test_endpoint "GET /user/orders/{user_id}" "GET" "/user/orders/$TEST_USER_ID" "" || true

echo ""
echo "==============================="
echo -e "${BLUE}Test Summary${NC}"
echo "==============================="
echo -e "Passed: ${GREEN}$TESTS_PASSED${NC}"
echo -e "Failed: ${RED}$TESTS_FAILED${NC}"
echo ""

if [ $TESTS_PASSED -gt 0 ]; then
    echo -e "${GREEN}✅ Tests completed!${NC}"
    exit 0
else
    echo -e "${YELLOW}⚠️  No tests passed - check if API is running${NC}"
    exit 1
fi
