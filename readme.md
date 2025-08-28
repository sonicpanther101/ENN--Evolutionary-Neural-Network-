### Simplified Pseudocode (Minimal Working Version)

```python
# Per time step
y = x + Δt * v + Δt² * a_ext
for n_iter iterations:
    for each vertex i (in parallel):
        f_i = - (M_i / Δt²) * (x_i - y_i)
        H_i = M_i / Δt²
        for each force j affecting vertex i:
            f_i -= k_j * C_j(x) * ∂C_j/∂x_i
            H_i += k_j * (∂C_j/∂x_i)^T * (∂C_j/∂x_i)
        Δx_i = H_i^{-1} * f_i
        x_i += Δx_i
v = (x - x_prev) / Δt
```

### Removed Features (Ordered by Difficulty to Reimplement)

1. **Warm Starting**  
   - Scaling down stiffness and dual variables from previous frames (Equation 19).  
   - *Difficulty: Easy* (simple parameter scaling).

2. **Progressive Stiffness Ramping for Soft Constraints**  
   - Gradually increasing stiffness for soft constraints using Equation 16.  
   - *Difficulty: Easy* (conditional stiffness updates).

3. **Collision Detection & Contact Persistence**  
   - Broad/narrow-phase collision detection and storing contact data across frames.  
   - *Difficulty: Medium* (requires spatial data structures and state management).

4. **Inequality Constraints & Clamping**  
   - Bounding Lagrange multipliers (Equation 13) and stiffness rescaling (Equation 14).  
   - *Difficulty: Medium* (requires force bounds and conditional logic).

5. **Frictional Contacts**  
   - Modeling static/dynamic friction cones and tangent constraints (Equation 15).  
   - *Difficulty: Hard* (requires non-linear constraints and state transitions).

6. **Augmented Lagrangian for Hard Constraints**  
   - Dual variable updates (Equation 11), stiffness ramping (Equation 12), and energy regularization (Equation 18).  
   - *Difficulty: Hard* (requires iterative dual updates and careful energy handling).

7. **Approximate SPD Hessians**  
   - Replacing indefinite Hessians with diagonal approximations (Section 3.5).  
   - *Difficulty: Hard* (requires spectral analysis and fallback strategies).

8. **Rigid Body Rotations**  
   - Handling quaternion-based updates and angular degrees of freedom (Equations 20–21).  
   - *Difficulty: Very Hard* (non-linear manifold optimization).

### Notes
- The minimal version assumes **soft constraints only** (no hard constraints or friction).
- Parallelization via vertex coloring is retained but simplified (no dynamic coloring updates).
- All features beyond basic soft-body VBD are stripped. Readding them requires incremental implementation in the order above.