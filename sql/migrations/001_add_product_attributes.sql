-- Migration: Add product attributes support
-- This adds flexible JSONB column for product variants/attributes
-- Examples: {"size": "42", "color": "black"}, {"storage": "256GB", "model": "Pro Max"}

-- Add attributes column to products table
ALTER TABLE products 
ADD COLUMN IF NOT EXISTS attributes JSONB DEFAULT NULL;

-- Add index for faster JSON queries
CREATE INDEX IF NOT EXISTS idx_products_attributes ON products USING GIN (attributes);

-- Add comment for documentation
COMMENT ON COLUMN products.attributes IS 'Flexible product attributes/variants in JSONB format. Example: {"size": "42", "color": "black", "storage": "256GB"}';

-- Migration is backward compatible:
-- - Existing products will have NULL attributes (simple products without variants)
-- - New products can use attributes field for variants
-- - No data loss or breaking changes
