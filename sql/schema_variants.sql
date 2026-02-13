-- Product Variants System
-- Complete redesign to support products with multiple variants
-- Each variant can have its own price, image, stock, description

-- Base products table (parent products)
CREATE TABLE IF NOT EXISTS products (
    id SERIAL PRIMARY KEY,
    shop_id INTEGER NOT NULL REFERENCES shops(id) ON DELETE CASCADE,
    product_id UUID DEFAULT gen_random_uuid() UNIQUE,
    title VARCHAR(255) NOT NULL,
    description TEXT,
    category VARCHAR(100),
    tags TEXT[],
    is_active BOOLEAN DEFAULT TRUE,
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
);

-- Product variants table (each variant has own price, image, stock, etc.)
CREATE TABLE IF NOT EXISTS product_variants (
    id SERIAL PRIMARY KEY,
    variant_id UUID DEFAULT gen_random_uuid() UNIQUE NOT NULL,
    product_id UUID NOT NULL REFERENCES products(product_id) ON DELETE CASCADE,
    sku VARCHAR(100),  -- Stock Keeping Unit (optional)
    title VARCHAR(255) NOT NULL,  -- Variant-specific title, e.g. "iPhone 15 Pro 256GB Black"
    price INTEGER NOT NULL CHECK (price > 0),
    currency VARCHAR(3) DEFAULT 'RUB',
    image_url TEXT,
    description TEXT,  -- Variant-specific description
    stock_count INTEGER DEFAULT 0 CHECK (stock_count >= 0),
    attributes JSONB DEFAULT '{}',  -- e.g. {"storage": "256GB", "color": "Black"}
    is_active BOOLEAN DEFAULT TRUE,
    is_default BOOLEAN DEFAULT FALSE,  -- Mark one variant as default for display
    sort_order INTEGER DEFAULT 0,  -- For custom ordering of variants
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
);

-- Indexes for performance
CREATE INDEX IF NOT EXISTS idx_products_shop_id ON products(shop_id);
CREATE INDEX IF NOT EXISTS idx_products_category ON products(category);
CREATE INDEX IF NOT EXISTS idx_variants_product_id ON product_variants(product_id);
CREATE INDEX IF NOT EXISTS idx_variants_attributes ON product_variants USING GIN (attributes);
CREATE INDEX IF NOT EXISTS idx_variants_sku ON product_variants(sku);

-- View for easy querying of products with their default variant
CREATE OR REPLACE VIEW products_with_default_variant AS
SELECT 
    p.id,
    p.product_id,
    p.shop_id,
    p.title as product_title,
    p.description as product_description,
    p.category,
    p.tags,
    p.is_active as product_active,
    p.created_at as product_created_at,
    p.updated_at as product_updated_at,
    v.variant_id,
    v.sku,
    v.title as variant_title,
    v.price,
    v.currency,
    v.image_url,
    v.description as variant_description,
    v.stock_count,
    v.attributes,
    v.is_active as variant_active,
    v.sort_order
FROM products p
LEFT JOIN LATERAL (
    SELECT *
    FROM product_variants
    WHERE product_id = p.product_id
    ORDER BY is_default DESC, sort_order ASC, created_at ASC
    LIMIT 1
) v ON true;

-- Function to update updated_at timestamp
CREATE OR REPLACE FUNCTION update_updated_at_column()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = NOW();
    RETURN NEW;
END;
$$ language 'plpgsql';

-- Triggers to auto-update updated_at
DROP TRIGGER IF EXISTS update_products_updated_at ON products;
CREATE TRIGGER update_products_updated_at
    BEFORE UPDATE ON products
    FOR EACH ROW
    EXECUTE FUNCTION update_updated_at_column();

DROP TRIGGER IF EXISTS update_product_variants_updated_at ON product_variants;
CREATE TRIGGER update_product_variants_updated_at
    BEFORE UPDATE ON product_variants
    FOR EACH ROW
    EXECUTE FUNCTION update_updated_at_column();

-- Comments for documentation
COMMENT ON TABLE products IS 'Base products table - parent products that group variants';
COMMENT ON TABLE product_variants IS 'Product variants - each variant has own price, image, stock, and attributes';
COMMENT ON COLUMN product_variants.attributes IS 'JSONB field storing variant-specific attributes like {"storage": "256GB", "color": "Black"}';
COMMENT ON COLUMN product_variants.is_default IS 'Marks the default variant to show when displaying product list';
COMMENT ON COLUMN product_variants.sort_order IS 'Custom ordering for variants (lower numbers first)';
