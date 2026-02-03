# Testing Guide

This guide explains how to test the ShopKit backend API.

## Prerequisites

- `curl` - Command-line tool for HTTP requests
- `jq` - JSON processor (optional, for pretty-printing)
- Running ShopKit backend instance

## Quick Test

The easiest way to test all endpoints is using the provided test script:

```bash
# Test local instance
./test_api.sh

# Test production instance
API_BASE=https://your-app.railway.app/api ./test_api.sh
```

## Manual Testing

### 1. Shop Endpoints

#### Get Shop by Token

```bash
curl http://localhost:8080/api/shops/token/my_shop_123
```

Expected response:
```json
{
  "id": 1,
  "shop_token": "my_shop_123",
  "title": "My Shop",
  "description": "Shop description",
  "currency": "RUB",
  "created_at": "2025-01-01T00:00:00Z",
  "products_count": 10
}
```

#### Create Shop

```bash
curl -X POST http://localhost:8080/api/shops \
  -H "Content-Type: application/json" \
  -d '{
    "owner_tg_id": 123456789,
    "title": "Coffee Shop",
    "description": "Best coffee in town",
    "currency": "RUB",
    "shop_token": "coffee_shop"
  }'
```

Expected response:
```json
{
  "success": true,
  "message": "Shop created successfully",
  "shop": {
    "id": 1,
    "shop_token": "coffee_shop",
    "title": "Coffee Shop",
    "created_at": "2025-01-01T00:00:00Z"
  }
}
```

### 2. Product Endpoints

#### Get Products

```bash
curl http://localhost:8080/api/products/1
```

Expected response:
```json
[
  {
    "product_id": "uuid-here",
    "title": "Espresso",
    "price": 150,
    "currency": "RUB",
    "image_url": "https://example.com/image.jpg",
    "description": "Strong coffee",
    "stock_count": 100,
    "category": "hot_drinks"
  }
]
```

### 3. Cart Endpoints

#### Save Cart

```bash
curl -X POST http://localhost:8080/api/cart \
  -H "Content-Type: application/json" \
  -d '{
    "telegram_user_id": 123456789,
    "shop_id": 1,
    "items": [
      {
        "product_id": "uuid-1",
        "title": "Coffee",
        "quantity": 2,
        "price": 250
      },
      {
        "product_id": "uuid-2",
        "title": "Tea",
        "quantity": 1,
        "price": 150
      }
    ]
  }'
```

Expected response:
```json
{
  "success": true,
  "updated_at": "2025-01-01T12:00:00Z"
}
```

#### Get Cart

```bash
curl "http://localhost:8080/api/cart/123456789?shop_id=1"
```

Expected response:
```json
{
  "telegram_user_id": 123456789,
  "shop_id": 1,
  "items": [
    {
      "product_id": "uuid-1",
      "title": "Coffee",
      "quantity": 2,
      "price": 250
    }
  ],
  "updated_at": "2025-01-01T12:00:00Z"
}
```

#### Clear Cart

```bash
curl -X DELETE "http://localhost:8080/api/cart/123456789?shop_id=1"
```

Expected response:
```json
{
  "success": true
}
```

### 4. Order Endpoints

#### Create Order

```bash
curl -X POST http://localhost:8080/api/orders \
  -H "Content-Type: application/json" \
  -d '{
    "shop_id": 1,
    "telegram_user_id": 123456789,
    "customer_name": "John Doe",
    "customer_phone": "+1234567890",
    "items": [
      {
        "product_id": "uuid-1",
        "title": "Coffee",
        "quantity": 2,
        "price": 250
      }
    ],
    "total_price": 500,
    "currency": "RUB"
  }'
```

Expected response:
```json
{
  "success": true,
  "order": {
    "order_id": "uuid-order",
    "shop_id": 1,
    "total_price": 500,
    "currency": "RUB",
    "status": "pending",
    "items": [...],
    "created_at": "2025-01-01T12:00:00Z"
  }
}
```

#### Get User Orders

```bash
curl http://localhost:8080/api/user/orders/123456789
```

Expected response:
```json
[
  {
    "order_id": "uuid-order",
    "shop_id": 1,
    "shop_title": "Coffee Shop",
    "shop_token": "coffee_shop",
    "total_price": 500,
    "currency": "RUB",
    "status": "pending",
    "items": [...],
    "created_at": "2025-01-01T12:00:00Z"
  }
]
```

## Testing with Postman

1. Import the endpoints into Postman
2. Set base URL: `http://localhost:8080/api`
3. Create requests for each endpoint
4. Save as a collection for reuse

## Error Responses

All errors follow this format:

```json
{
  "error": "Error message",
  "details": "Optional detailed error information"
}
```

Common HTTP status codes:
- `200 OK` - Success
- `201 Created` - Resource created successfully
- `400 Bad Request` - Invalid request data
- `404 Not Found` - Resource not found
- `500 Internal Server Error` - Server error

## Integration Testing

For integration testing with your frontend:

1. Start the backend: `./shopkit`
2. Use the TypeScript SDK from `FRONTEND_API.md`
3. Test all user flows:
   - Browse products
   - Add to cart
   - Save cart
   - Checkout
   - View order history

## Performance Testing

### Load Testing with Apache Bench

```bash
# Test get products endpoint
ab -n 1000 -c 10 http://localhost:8080/api/products/1

# Test cart endpoint
ab -n 1000 -c 10 -p cart.json -T application/json \
  http://localhost:8080/api/cart
```

### Expected Performance

- Get requests: < 50ms
- Post requests: < 100ms
- Database queries: < 20ms
- Throughput: > 1000 req/s

## Troubleshooting

### Database Connection Issues

```bash
# Check database is running
psql -U postgres -d shopkit -c "SELECT 1"

# Check database connection in logs
tail -f shopkit/build/logs/shopkit.log
```

### API Not Responding

```bash
# Check if service is running
ps aux | grep shopkit

# Check port is open
netstat -tuln | grep 8080

# Test with verbose curl
curl -v http://localhost:8080/api/shops/token/test
```

### Invalid JSON Errors

Make sure your JSON is valid:

```bash
# Validate JSON
echo '{"test": "data"}' | jq .

# Use proper Content-Type header
curl -H "Content-Type: application/json" ...
```

## Continuous Integration Testing

Example GitHub Actions workflow:

```yaml
name: API Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    
    services:
      postgres:
        image: postgres:14
        env:
          POSTGRES_DB: shopkit
          POSTGRES_PASSWORD: postgres
        options: >-
          --health-cmd pg_isready
          --health-interval 10s
          --health-timeout 5s
          --health-retries 5
    
    steps:
      - uses: actions/checkout@v2
      
      - name: Build
        run: |
          cd shopkit
          mkdir build && cd build
          cmake ..
          make
      
      - name: Start server
        run: ./shopkit/build/shopkit &
        
      - name: Run tests
        run: ./test_api.sh
```

## Security Testing

Run security scans:

```bash
# SQL injection test
curl -X POST http://localhost:8080/api/cart \
  -H "Content-Type: application/json" \
  -d '{"telegram_user_id": "1; DROP TABLE carts--", "shop_id": 1, "items": []}'

# Should be properly escaped and rejected
```

Expected: Proper error handling without SQL execution.

## Next Steps

After testing:
1. Review logs for errors
2. Check database for data consistency
3. Monitor performance metrics
4. Deploy to production
5. Set up monitoring and alerting
