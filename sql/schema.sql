CREATE TABLE IF NOT EXISTS shops (
                                     id SERIAL PRIMARY KEY,
                                     shop_token VARCHAR(64) UNIQUE NOT NULL,
    owner_tg_id BIGINT NOT NULL,
    title VARCHAR(255) NOT NULL,
    description TEXT,
    currency VARCHAR(3) DEFAULT 'RUB',
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
    );

CREATE TABLE IF NOT EXISTS products (
                                        id SERIAL PRIMARY KEY,
                                        shop_id INTEGER NOT NULL REFERENCES shops(id) ON DELETE CASCADE,
    product_id UUID DEFAULT gen_random_uuid() UNIQUE,
    title VARCHAR(255) NOT NULL,
    price INTEGER NOT NULL CHECK (price > 0),
    currency VARCHAR(3) DEFAULT 'RUB',
    image_url TEXT,
    description TEXT,
    stock_count INTEGER DEFAULT 0,
    is_active BOOLEAN DEFAULT TRUE,
    category VARCHAR(100),
    tags TEXT[],
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
    );

CREATE TABLE IF NOT EXISTS orders (
                                      id SERIAL PRIMARY KEY,
                                      shop_id INTEGER NOT NULL REFERENCES shops(id) ON DELETE CASCADE,
    order_id UUID DEFAULT gen_random_uuid() UNIQUE,
    customer_name VARCHAR(255),
    telegram_user_id BIGINT,
    customer_phone VARCHAR(20),
    items JSONB NOT NULL,
    total_price INTEGER NOT NULL,
    currency VARCHAR(3) DEFAULT 'RUB',
    status VARCHAR(50) DEFAULT 'pending',
    payment_method VARCHAR(50),
    notes TEXT,
    created_at TIMESTAMP DEFAULT NOW()
    );

CREATE TABLE IF NOT EXISTS users (
                                     id SERIAL PRIMARY KEY,
                                     telegram_user_id BIGINT UNIQUE NOT NULL,
                                     username VARCHAR(255),
    first_name VARCHAR(100),
    last_name VARCHAR(100),
    phone VARCHAR(20),
    created_at TIMESTAMP DEFAULT NOW()
    );

CREATE TABLE IF NOT EXISTS shop_admins (
    shop_id INTEGER NOT NULL REFERENCES shops(id) ON DELETE CASCADE,
    telegram_user_id BIGINT NOT NULL,
    role VARCHAR(50) DEFAULT 'manager',
    permissions JSONB DEFAULT '{}',
    created_at TIMESTAMP DEFAULT NOW(),
    PRIMARY KEY (shop_id, telegram_user_id)
    );

CREATE INDEX IF NOT EXISTS idx_products_shop_id ON products(shop_id);
CREATE INDEX IF NOT EXISTS idx_orders_shop_id ON orders(shop_id);
CREATE INDEX IF NOT EXISTS idx_orders_tg_user ON orders(telegram_user_id);
CREATE INDEX IF NOT EXISTS idx_shops_token ON shops(shop_token);
CREATE INDEX IF NOT EXISTS idx_shops_owner ON shops(owner_tg_id);