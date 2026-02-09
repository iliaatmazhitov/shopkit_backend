# ShopKit Backend

> **🚀 Production Status:** LIVE at https://shopkit-backend.onrender.com/api

Production-ready backend for Telegram Mini App e-commerce platform built with C++ and Drogon framework.

## For Frontend Developers

**📋 [API Endpoints Reference](./API_ENDPOINTS.md)** ← Начни здесь! Простой справочник всех endpoints

**📚 [Complete Integration Guide](./FRONTEND_COMPLETE_GUIDE.md)** - полное руководство с TypeScript, React hooks и компонентами

Quick start for building Telegram Mini Apps:
- **Production API:** `https://shopkit-backend.onrender.com/api`
- **13 API endpoints** - все документированы с примерами запросов/ответов
- **Full TypeScript API client** with error handling and retry logic
- **Production-ready React hooks** (useCart, useShop, useProducts, useOrders)
- **Complete page components** ready to copy and use
- **Telegram Mini App examples** with deep linking
- **Testing guide** with real production examples

**Documentation:**
- **[📋 API_ENDPOINTS.md](./API_ENDPOINTS.md)** - простой справочник: адрес + метод + JSON
- **[📖 FRONTEND_COMPLETE_GUIDE.md](./FRONTEND_COMPLETE_GUIDE.md)** - полное руководство по интеграции

## Features

### Shop Management
- Create and manage shops
- Unique shop tokens for easy access
- Multi-shop support

### Product Management
- Add, update, and delete products
- Product catalog with images
- Category and tag support
- Stock management

### Shopping Cart (NEW)
- Server-side cart persistence
- Multi-shop cart support
- Automatic cart synchronization

### Order Management (NEW)
- Create orders with customer details
- Order history across all shops
- Order status tracking

### User Features
- Telegram user integration
- Complete order history
- Cart persistence across sessions

## Tech Stack

- **Framework**: [Drogon](https://github.com/drogonframework/drogon) (C++ web framework)
- **Database**: PostgreSQL
- **Deployment**: Railway / Docker
- **API**: RESTful JSON API

## Quick Start

### Prerequisites

- Ubuntu 22.04 (or similar Linux distribution)
- CMake 3.5+
- C++17 or higher
- PostgreSQL
- Drogon framework

### Installation

1. **Install Dependencies**

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake git libpq-dev libjsoncpp-dev libssl-dev zlib1g-dev libbrotli-dev uuid-dev
```

2. **Install Drogon Framework**

```bash
git clone https://github.com/drogonframework/drogon
cd drogon
git submodule update --init
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
```

3. **Clone and Build ShopKit**

```bash
git clone https://github.com/iliaatmazhitov/shopkit_backend.git
cd shopkit_backend/shopkit
mkdir build && cd build
cmake ..
make -j$(nproc)
```

4. **Setup Database**

```bash
# Create PostgreSQL database
createdb shopkit

# Apply schema
psql shopkit < ../../sql/schema.sql
```

5. **Configure Database Connection**

Edit `config.json`:

```json
{
  "db_clients": [
    {
      "name": "default",
      "rdbms": "postgresql",
      "host": "127.0.0.1",
      "port": 5432,
      "dbname": "shopkit",
      "user": "your_user",
      "passwd": "your_password"
    }
  ]
}
```

6. **Run the Application**

```bash
./shopkit
```

The API will be available at `http://localhost:8080/api`

## API Documentation

See [FRONTEND_API.md](./FRONTEND_API.md) for complete API documentation and TypeScript integration guide.

### Quick API Overview

#### Shop Endpoints
- `GET /api/shops/token/{shop_token}` - Get shop by token
- `POST /api/shops` - Create new shop
- `PUT /api/shops/{shop_token}` - Update shop

#### Product Endpoints
- `GET /api/products/{shop_id}` - Get all products
- `GET /api/products/detail/{product_id}` - Get product details
- `POST /api/admin/products` - Create product
- `PUT /api/admin/products/{product_id}` - Update product
- `DELETE /api/admin/products/{product_id}` - Delete product

#### Cart Endpoints (NEW)
- `POST /api/cart` - Save cart
- `GET /api/cart/{telegram_user_id}?shop_id={shop_id}` - Get cart
- `DELETE /api/cart/{telegram_user_id}?shop_id={shop_id}` - Clear cart

#### Order Endpoints (NEW)
- `POST /api/orders` - Create order
- `GET /api/user/orders/{telegram_user_id}` - Get all user orders

## Deployment

See [DEPLOY.md](./DEPLOY.md) for complete deployment instructions.

### Quick Deploy to Railway

```bash
# Install Railway CLI
npm install -g @railway/cli

# Login and initialize
railway login
cd shopkit_backend
railway init

# Add PostgreSQL
railway add postgresql

# Deploy
railway up

# Apply schema
railway run psql $DATABASE_URL -f sql/schema.sql

# Get your URL
railway domain
```

## Database Schema

The application uses PostgreSQL with the following main tables:

- **shops** - Shop information and configuration
- **products** - Product catalog
- **orders** - Order records
- **carts** - User shopping carts (NEW)
- **users** - User profiles
- **shop_admins** - Admin permissions

See `sql/schema.sql` for complete schema.

## Testing

```bash
# Test shop endpoint
curl http://localhost:8080/api/shops/token/test_shop

# Test cart save
curl -X POST http://localhost:8080/api/cart \
  -H "Content-Type: application/json" \
  -d '{
    "telegram_user_id": 123456789,
    "shop_id": 1,
    "items": [
      {"product_id":"uuid","title":"Coffee","quantity":2,"price":250}
    ]
  }'

# Test cart retrieval
curl "http://localhost:8080/api/cart/123456789?shop_id=1"

# Test user orders
curl http://localhost:8080/api/user/orders/123456789
```

## Project Structure

```
shopkit_backend/
├── README.md
├── FRONTEND_API.md       # TypeScript integration guide
├── DEPLOY.md            # Deployment instructions
├── Dockerfile           # Docker configuration
├── sql/
│   └── schema.sql       # Database schema
├── shopkit/
│   ├── controllers/     # API endpoints
│   │   ├── ShopController.cc
│   │   ├── ProductController.cc
│   │   ├── OrderController.cc
│   │   └── CartController.cc
│   ├── models/          # Data models
│   ├── filters/         # Middleware
│   ├── config.json      # Configuration
│   └── main.cc          # Entry point
```

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License.

## Support

For issues and questions:
- Create an issue on GitHub
- Contact: iliaatmazhitov@example.com

## Roadmap

- [x] Basic shop and product management
- [x] Server-side cart persistence
- [x] User order history
- [ ] Payment integration (Telegram Stars, Stripe)
- [ ] Webhook notifications
- [ ] Analytics dashboard
- [ ] Multi-language support
- [ ] Image optimization
- [ ] Real-time inventory updates

## Acknowledgments

- [Drogon Framework](https://github.com/drogonframework/drogon) - High-performance C++ web framework
- PostgreSQL - Reliable database
- Railway - Easy deployment platform
